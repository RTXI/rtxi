/*
 Copyright (C) 2011 Georgia Institute of Technology

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

/*
 * IIRfilter
 * Computes IIR filter coefficients
 *
 */

#include <IIRfilter.h>
#include <math.h>
#include <algorithm>
#include <numeric>
#include <time.h>
#include "../include/DSP/log2.h"

#include <qcheckbox.h>
#include <qdatetime.h>
#include <qfile.h>
#include <qgridview.h>
#include <qhbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qcombobox.h>

#include <qpushbutton.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qtextstream.h>
#include <sys/stat.h>

//create plug-in
extern "C" Plugin::Object *
createRTXIPlugin(void)
{
  return new IIRfilter(); // Change the name of the plug-in here
}

//set up parameters/inputs/outputs derived from defaultGUIModel, calls for initialization, creation, update, and refresh of GUI
static DefaultGUIModel::variable_t vars[] =
  {
    { "Input", "Input to Filter", DefaultGUIModel::INPUT, },
    { "Output", "Output of Filter", DefaultGUIModel::OUTPUT },
    { "Filter Order", "Filter Order", DefaultGUIModel::PARAMETER
        | DefaultGUIModel::INTEGER, },
    { "Passband Ripple (dB)", "Passband Ripple (dB)",
        DefaultGUIModel::PARAMETER | DefaultGUIModel::DOUBLE, },
    { "Passband Edge (Hz)", "Passband Edge (Hz)", DefaultGUIModel::PARAMETER
        | DefaultGUIModel::DOUBLE, },
    { "Stopband Ripple (dB)", "Stopband Ripple (dB)",
        DefaultGUIModel::PARAMETER | DefaultGUIModel::DOUBLE, },
    { "Stopband Edge (Hz)", "Stopband Edge (Hz)", DefaultGUIModel::PARAMETER
        | DefaultGUIModel::DOUBLE, },
    { "Input quantizing factor", "Bits eg. 10, 12, 16",
        DefaultGUIModel::PARAMETER | DefaultGUIModel::INTEGER, },
    { "Coefficients quantizing factor", "Bits eg. 10, 12, 16",
        DefaultGUIModel::PARAMETER | DefaultGUIModel::INTEGER, },
    { "Time (s)", "Time (s)", DefaultGUIModel::STATE, }, };

static size_t num_vars = sizeof(vars) / sizeof(DefaultGUIModel::variable_t);

IIRfilter::IIRfilter(void) :
  DefaultGUIModel("IIR Filter", ::vars, ::num_vars)
{
  QWhatsThis::add(
      this,
      "<p><b>IIR Filter:</b><br>This plugin computes filter coefficients for three types of IIR filters. "
        "They require the following parameters: <br><br>"
        "Butterworth: passband edge <br>"
        "Chebyshev: passband ripple, passband edge, ripple bw_norm <br>"
        "Elliptical: passband ripple, stopband ripple, passband edge, stopband edge <br>"
        "Bessel: passband edge<br><br>"
        "Since this plug-in computes new filter coefficients whenever you change the parameters, you should not"
        "change any settings during real-time.</p>");

  initParameters();
  createGUI(vars, num_vars);
  update(INIT);
  refresh(); // refresh the GUI
  printf("\nStarting IIR filter:\n"); // prints to terminal
}

IIRfilter::~IIRfilter(void)
{
}

//execute, the code block that actually does the signal processing
void
IIRfilter::execute(void)
{
  systime = count * dt; // current time, s
  output(0) = filter_implem->ProcessSample(input(0));

  count++; // increment count to measure time
  return;
}

void
IIRfilter::update(DefaultGUIModel::update_flags_t flag)
{
  switch (flag)
    {
  case INIT:
    setParameter("Filter Order", QString::number(filter_order));
    setParameter("Passband Ripple (dB)", QString::number(passband_ripple));
    setParameter("Passband Edge (Hz)", QString::number(passband_edge));
    setParameter("Stopband Ripple (dB)", QString::number(stopband_ripple));
    setParameter("Stopband Edge (Hz)", QString::number(stopband_edge));
    setParameter("Input quantizing factor", QString::number(ilog2(
        input_quan_factor)));
    setParameter("Coefficients quantizing factor", QString::number(ilog2(
        coeff_quan_factor)));
    setState("Time (s)", systime);
    filterType->setCurrentItem(filter_type);
    break;
  case MODIFY:
    filter_order = int(getParameter("Filter Order").toDouble());
    passband_ripple = getParameter("Passband Ripple (dB)").toDouble();
    passband_edge = getParameter("Passband Edge (Hz)").toDouble();
    stopband_ripple = getParameter("Stopband Ripple (dB)").toDouble();
    stopband_edge = getParameter("Stopband Edge (Hz)").toDouble();
    filter_type = filter_t(filterType->currentItem());
    stopband_edge *= TWO_PI; // because frequency specified in Hz, not rad/s
    passband_edge *= TWO_PI;
    input_quan_factor = 2 ^ getParameter("Input quantizing factor").toInt(); // quantize input to 12 bits
    coeff_quan_factor = 2
        ^ getParameter("Coefficients quantizing factor").toInt(); // quantize filter coefficients to 12 bits
    bookkeep();
    makeFilter();
    break;
  case PAUSE:
    output(0) = 0; // stop command in case pause occurs in the middle of command
    printf("Protocol paused.\n");
    break;
  case UNPAUSE:
    bookkeep();
    printf("Protocol started.\n");
    break;
  case PERIOD:
    dt = RT::System::getInstance()->getPeriod() * 1e-9; // s
  default:
    break;
    }
}

// custom functions, as defined in the header file

void
IIRfilter::initParameters()
{
  dt = RT::System::getInstance()->getPeriod() * 1e-9; // s
  filter_type = BUTTER;
  filter_order = 10;
  passband_ripple = 3;
  passband_edge = 60;
  stopband_ripple = 60;
  stopband_edge = 200;
  ripple_bw_norm = 0;
  predistort_enabled = true;
  quant_enabled = false;
  input_quan_factor = 4096; // quantize input to 12 bits
  coeff_quan_factor = 4096; // quantize filter coefficients to 12 bits
  makeFilter();
  bookkeep();
}

void
IIRfilter::bookkeep()
{
  count = 0;
  systime = 0;
  printf("input quan factor: %i, coeff quan factor: %i\n", input_quan_factor,
      coeff_quan_factor);
  printf("input quan bits: %i, coeff quan bits: %i\n",
      ilog2(input_quan_factor), ilog2(coeff_quan_factor));
}

void
IIRfilter::updateFilterType(int index)
{
  if (index == 0)
    {
      filter_type = BUTTER;
      printf("Filter type now set to BUTTERWORTH\n");
      normType->setEnabled(false);
      makeFilter();
    }
  else if (index == 1)
    {
      filter_type = CHEBY;
      printf("Filter type now set to CHEBYSHEV\n");
      normType->changeItem("3 dB attenuation at passband", 0);
      normType->setEnabled(true);
      makeFilter();
    }
  else if (index == 2)
    {
      filter_type = ELLIP;
      printf("Filter type now set to ELLIPTICAL\n");
      normType->setEnabled(false);
      makeFilter();
    }
  else if (index == 3)
    {
      filter_type = BESSEL;
      printf("Filter type now set to BESSEL\n");
      normType->changeItem("Unit delay at w=0", 0);
      normType->setEnabled(true);
      makeFilter();
    }
}

void
IIRfilter::updateNormType(int index)
{
  ripple_bw_norm = index;
  switch (filter_type)
    {
  case CHEBY:
    if (ripple_bw_norm)
      printf("Chebyshev normalization now set to ripple bandwidth\n");
    else
      printf(
          "Chebyshev normalization now set to 3 dB attenuation at the passband edge\n");

    break;
  case BESSEL:
    if (ripple_bw_norm)
      printf("Bessel normalization now set to unit delay at zero frequency\n");
    else
      printf(
          "Bessel normalization now set to 3 dB attenuation at the passband edge\n");
    break;
  case ELLIP:
  case BUTTER:
  default:
    break;
    }
  makeFilter();
}

void
IIRfilter::makeFilter()
{
  int upper_summation_limit = 5;
  switch (filter_type)
    {
  case BUTTER:
    analog_filter = new ButterworthTransFunc(filter_order);
    analog_filter->LowpassDenorm(passband_edge);
    break;
  case CHEBY:
    analog_filter = new ChebyshevTransFunc(filter_order, passband_ripple,
        ripple_bw_norm);
    analog_filter->LowpassDenorm(passband_edge);
    break;
  case ELLIP:
    analog_filter = new EllipticalTransFunc(filter_order, passband_ripple,
        stopband_ripple, passband_edge, stopband_edge, upper_summation_limit);
    break;
  case BESSEL:
    analog_filter = new BesselTransFunc(filter_order, passband_edge,
        ripple_bw_norm);
    break;
    } // end of switch on window_shape
  if (predistort_enabled)
    analog_filter->FrequencyPrewarp(dt);

  filter_design = BilinearTransf(analog_filter, dt);

  if (quant_enabled)
    {
      filter_implem = new DirectFormIir(filter_design->GetNumNumerCoeffs(),
          filter_design->GetNumDenomCoeffs(),
          filter_design->GetNumerCoefficients(),
          filter_design->GetDenomCoefficients(), coeff_quan_factor,
          input_quan_factor);
    }
  else
    {
      filter_implem = new UnquantDirectFormIir(
          filter_design->GetNumNumerCoeffs(),
          filter_design->GetNumDenomCoeffs(),
          filter_design->GetNumerCoefficients(),
          filter_design->GetDenomCoefficients());
    }

}

void
IIRfilter::saveIIRData()
{
  QFileDialog* fd = new QFileDialog(this, "Save File As", TRUE);
  fd->setMode(QFileDialog::AnyFile);
  fd->setViewMode(QFileDialog::Detail);
  QString fileName;
  if (fd->exec() == QDialog::Accepted)
    {
      fileName = fd->selectedFile();

      if (OpenFile(fileName))
        {
          //                    stream.setPrintableData(true);
          switch (filter_type)
            {
          case BUTTER:
            stream << QString("BUTTERWORTH order=") << (int) filter_order
                << " passband edge=" << (double) passband_edge;
            break;
          case CHEBY:
            stream << QString("CHEBYSHEV order=") << (int) filter_order
                << " passband ripple=" << (double) passband_ripple
                << " passband edge=" << (double) passband_edge;
            if (ripple_bw_norm == 0)
              stream << " with 3 dB bandwidth normalization";
            else
              stream << " with ripple bandwidth normalization";
            break;
          case ELLIP:
            stream << QString("ELLIPTICAL order=") << (int) filter_order
                << " passband ripple=" << (double) passband_ripple
                << " passband edge=" << (double) passband_edge
                << " stopband ripple=" << (double) stopband_ripple
                << " stopband edge=" << (double) stopband_edge;
            break;
          case BESSEL:
            stream << QString("BESSEL order=") << (int) filter_order
                << " passband edge=" << (double) passband_edge;
            if (ripple_bw_norm == 0)
              stream << " with 3 dB bandwidth normalization";
            else
              stream << " unit delay at zero frequency";
            break;
            }
          stream << QString(" \n");

          double *numer_coeff = new double[filter_design->GetNumNumerCoeffs()];
          double *denom_coeff = new double[filter_design->GetNumDenomCoeffs()
              + 1];
          numer_coeff = filter_design->GetNumerCoefficients();
          denom_coeff = filter_design->GetDenomCoefficients();

          printf("\nFilter numerator coefficients:\n");
          stream << QString("Filter numerator coefficients:\n");
          for (int i = 0; i < filter_design->GetNumNumerCoeffs(); i++)
            {
              printf("%f\n", numer_coeff[i]);
              stream << QString("numer_coeff[") << i << "] = "
                  << (double) numer_coeff[i] << "\n";
            }
          printf("Filter denominator coefficients:\n");
          stream << QString("Filter denominator coefficients:\n");
          for (int i = 0; i < filter_design->GetNumDenomCoeffs() + 1; i++)
            {
              printf("%f\n", denom_coeff[i]);
              stream << QString("denom_coeff[") << i << "] = "
                  << (double) denom_coeff[i] << "\n";
            }
          dataFile.close();
          printf("File closed.\n");
        }
      else
        {
          QMessageBox::information(this, "IIR filter: Save filter parameters",
              "There was an error writing to this file. You can view\n"
                "the parameters in the terminal.\n");
        }
    }
}

bool
IIRfilter::OpenFile(QString FName)
{
  dataFile.setName(FName);
  if (dataFile.exists())
    {
      switch (QMessageBox::warning(this, "IIR filter", tr(
          "This file already exists: %1.\n").arg(FName), "Overwrite", "Append",
          "Cancel", 0, 2))
        {
      case 0: // overwrite
        dataFile.remove();
        if (!dataFile.open(IO_Raw | IO_WriteOnly))
          {
            return false;
          }
        break;
      case 1: // append
        if (!dataFile.open(IO_Raw | IO_WriteOnly | IO_Append))
          {
            return false;
          }
        break;
      case 2: // cancel
        return false;
        break;
        }
    }
  else
    {
      if (!dataFile.open(IO_Raw | IO_WriteOnly))
        return false;
    }
  stream.setDevice(&dataFile);
  //    stream.setPrintableData(false); // write binary
  printf("File opened: %s\n", FName.latin1());
  return true;
}

//create the GUI components
void
IIRfilter::createGUI(DefaultGUIModel::variable_t *var, int size)
{
  QBoxLayout *layout = new QHBoxLayout(this); // overall GUI layout

  // Left side GUI
  QBoxLayout *leftlayout = new QVBoxLayout();
  QPushButton *saveDataButton = new QPushButton("Save IIR Coefficients", this);
  leftlayout->addWidget(saveDataButton);

  QObject::connect(saveDataButton, SIGNAL(clicked()), this, SLOT(saveIIRData()));
  QToolTip::add(saveDataButton,
      "Save filter parameters and coefficients to a file");

  QGridLayout *optionLayout = new QGridLayout(leftlayout, 2, 2);

  QLabel *filterLabel = new QLabel("Type of Filter:", this);
  filterType = new QComboBox(FALSE, this, "Filter Type");
  QToolTip::add(filterType, "IIR filter.");
  filterType->insertItem("Butterworth");
  filterType->insertItem("Chebyshev");
  filterType->insertItem("Elliptical");
  filterType->insertItem("Bessel");
  optionLayout->addWidget(filterLabel, 1, 0);
  optionLayout->addWidget(filterType, 1, 1);
  QObject::connect(filterType,SIGNAL(activated(int)), this, SLOT(updateFilterType(int)));

  QLabel *normTypeLabel = new QLabel("Type of Chebyshev normalization:", this);
  normType = new QComboBox(FALSE, this, "Chebyshev Normalization");
  normType->insertItem("3 dB attenuation at passband");
  normType->insertItem("Ripple bandwidth");
  QToolTip::add(normType, "Type of Chebyshev normalization");
  optionLayout->addWidget(normTypeLabel, 0, 0);
  optionLayout->addWidget(normType, 0, 1);
  QObject::connect(normType,SIGNAL(activated(int)), this, SLOT(updateNormType(int)));
  normType->setEnabled(false);

  // Add custom left side GUI components to layout above default_gui_model components
  //    leftlayout->addLayout(optionLayout);

  // Create default_gui_model GUI DO NOT EDIT
  QScrollView *sv = new QScrollView(this);
  sv->setResizePolicy(QScrollView::AutoOneFit);
  sv->setHScrollBarMode(QScrollView::AlwaysOff);
  leftlayout->addWidget(sv);

  QWidget *viewport = new QWidget(sv->viewport());
  sv->addChild(viewport);
  QGridLayout *scrollLayout = new QGridLayout(viewport, 1, 2);
  //loop through to create GUI buttons/text boxes
  size_t nstate = 0, nparam = 0, nevent = 0, ncomment = 0;
  for (size_t i = 0; i < num_vars; i++)
    {
      if (vars[i].flags & (PARAMETER | STATE | EVENT | COMMENT))
        {
          param_t param;

          param.label = new QLabel(vars[i].name, viewport);
          scrollLayout->addWidget(param.label, parameter.size(), 0);
          param.edit = new DefaultGUILineEdit(viewport);
          scrollLayout->addWidget(param.edit, parameter.size(), 1);

          QToolTip::add(param.label, vars[i].description);
          QToolTip::add(param.edit, vars[i].description);

          if (vars[i].flags & PARAMETER)
            {
              if (vars[i].flags & DOUBLE)
                {
                  param.edit->setValidator(new QDoubleValidator(param.edit));
                  param.type = PARAMETER | DOUBLE;
                }
              else if (vars[i].flags & UINTEGER)
                {
                  QIntValidator *validator = new QIntValidator(param.edit);
                  param.edit->setValidator(validator);
                  validator->setBottom(0);
                  param.type = PARAMETER | UINTEGER;
                }
              else if (vars[i].flags & INTEGER)
                {
                  param.edit->setValidator(new QIntValidator(param.edit));
                  param.type = PARAMETER | INTEGER;
                }
              else
                param.type = PARAMETER;
              param.index = nparam++;
              param.str_value = new QString;
            }
          else if (vars[i].flags & STATE)
            {
              param.edit->setReadOnly(true);
              param.type = STATE;
              param.index = nstate++;
            }
          else if (vars[i].flags & EVENT)
            {
              param.edit->setReadOnly(true);
              param.type = EVENT;
              param.index = nevent++;
            }
          else if (vars[i].flags & COMMENT)
            {
              param.type = COMMENT;
              param.index = ncomment++;
            }

          parameter[vars[i].name] = param;
        }
    }

  QVBox *optionBox = new QVBox(this);
  QHBox *optionRow1 = new QHBox(optionBox);
  QCheckBox *predistortCheckBox = new QCheckBox("Predistort frequencies",
      optionRow1);
  QHBox *optionRow2 = new QHBox(optionBox);
  QCheckBox *quantizeCheckBox = new QCheckBox(
      "Quantize input and coefficients", optionRow2);
  QObject::connect(predistortCheckBox,SIGNAL(toggled(bool)),this,SLOT(togglePredistort(bool)));
  QObject::connect(quantizeCheckBox,SIGNAL(toggled(bool)),this,SLOT(toggleQuantize(bool)));
  QToolTip::add(predistortCheckBox,
      "Predistort frequencies for bilinear transform");
  QToolTip::add(quantizeCheckBox, "Quantize input and coefficients");

  leftlayout->addWidget(optionBox);

  QHBox *utilityBox = new QHBox(this);
  pauseButton = new QPushButton("Pause", utilityBox);
  pauseButton->setToggleButton(true);
  QObject::connect(pauseButton, SIGNAL(toggled(bool)), this, SLOT(pause(bool)));
  QObject::connect(pauseButton, SIGNAL(toggled(bool)), saveDataButton, SLOT(setEnabled(bool)));
  QPushButton *modifyButton = new QPushButton("Modify", utilityBox);
  QObject::connect(modifyButton, SIGNAL(clicked(void)), this, SLOT(modify(void)));
  QPushButton *unloadButton = new QPushButton("Unload", utilityBox);
  QObject::connect(unloadButton, SIGNAL(clicked(void)), this, SLOT(exit(void)));
  QObject::connect(pauseButton, SIGNAL(toggled(bool)), modifyButton, SLOT(setEnabled(bool)));
  QToolTip::add(pauseButton, "Start/Stop filter");
  QToolTip::add(modifyButton, "Commit changes to parameter values");
  QToolTip::add(unloadButton, "Close plug-in");

  // add custom components to layout below default_gui_model components
  leftlayout->addWidget(utilityBox);
  // Add left and right side layouts to the overall layout
  layout->addLayout(leftlayout);
  //    layout->setResizeMode(QLayout::Fixed);

  // set GUI refresh rate
  show();

}

void
IIRfilter::togglePredistort(bool on)
{
  predistort_enabled = on;
}

void
IIRfilter::toggleQuantize(bool on)
{
  quant_enabled = on;
}
