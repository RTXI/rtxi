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
 * FIRwindow
 * Computes FIR filter coefficients using the window method and does straight convolution with an input signal.
 *
 */

#include <FIRwindow.h>
#include <math.h>
#include <algorithm>
#include <numeric>
#include <time.h>

#include <qdatetime.h>
#include <qfile.h>
#include <qgridview.h>
#include <qhbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qcombobox.h>

#include <qpushbutton.h>
#include <qtimer.h>
#include <qtooltip.h>
#include <qvalidator.h>
#include <qvbox.h>
#include <qwhatsthis.h>
#include <qtextstream.h>
#include <sys/stat.h>

//create plug-in
extern "C" Plugin::Object *
createRTXIPlugin(void)
{
  return new FIRwindow(); // Change the name of the plug-in here
}

//set up parameters/inputs/outputs derived from defaultGUIModel, calls for initialization, creation, update, and refresh of GUI
static DefaultGUIModel::variable_t
    vars[] =
      {
            { "Input", "Input to Filter", DefaultGUIModel::INPUT, },
            { "Output", "Output of Filter", DefaultGUIModel::OUTPUT },
            { "# Taps", "Number of Filter Taps (Odd Number)",
                DefaultGUIModel::PARAMETER | DefaultGUIModel::INTEGER, },
            {
                "Frequency 1 (Hz)",
                "Cut off Frequency #1 as Fraction of Pi, used for Lowpass/Highpass",
                DefaultGUIModel::PARAMETER | DefaultGUIModel::DOUBLE, },
            {
                "Frequency 2 (Hz)",
                "Cut off Frequency #1 as Fraction of Pi, not used for Lowpass/Highpass",
                DefaultGUIModel::PARAMETER | DefaultGUIModel::DOUBLE, },
            { "Chebyshev (dB)", "Attenuation Parameter for Chebyshev Window",
                DefaultGUIModel::PARAMETER | DefaultGUIModel::DOUBLE, },
            { "Kaiser Alpha", "Attenuation Parameter for Kaiser Window",
                DefaultGUIModel::PARAMETER | DefaultGUIModel::DOUBLE, },
            { "Time (s)", "Time (s)", DefaultGUIModel::STATE, }, };

static size_t num_vars = sizeof(vars) / sizeof(DefaultGUIModel::variable_t);

FIRwindow::FIRwindow(void) :
  DefaultGUIModel("FIR Window", ::vars, ::num_vars)
{
  QWhatsThis::add(
      this,
      "<p><b>FIR Window:</b><br>This plugin computes FIR filter coefficients using the window method "
        "given the number of taps desired and the cutoff frequencies. For a lowpass or highpass filter, use the "
        "Freq 1 parameter. For a bandpass or bandstop filter, use both frequencies to define the frequency band. "
        "Since this plug-in computes new filter coefficients whenever you change the parameters, you should not "
        "change any settings during real-time.</p>");

  initParameters();
  createGUI(vars, num_vars);
  update( INIT);
  refresh(); // refresh the GUI
  printf("\nStarting FIR window filter:\n"); // prints to terminal
}

FIRwindow::~FIRwindow(void)
{
}

//execute, the code block that actually does the signal processing
void
FIRwindow::execute(void)
{
  signalin.push_back(input(0));
  systime = count * dt; // current time, s
  out = 0;
  for (n = num_taps; n < 2 * num_taps; n++)
    {
      out += h3[n] * signalin[n];
    }
  output(0) = out;

  count++; // increment count to measure time
  return;
}

void
FIRwindow::update(DefaultGUIModel::update_flags_t flag)
{
  switch (flag)
    {
  case INIT:
    setParameter("# Taps", QString::number(num_taps));
    setParameter("Frequency 1 (Hz)", QString::number(lambda1));
    setParameter("Frequency 2 (Hz)", QString::number(lambda2));
    setParameter("Kaiser Alpha", QString::number(Kalpha));
    setParameter("Chebyshev (dB)", QString::number(Calpha));
    setState("Time (s)", systime);
    windowShape->setCurrentItem(window_shape);
    filterType->setCurrentItem(filter_type);
    break;
  case MODIFY:
    num_taps = int(getParameter("# Taps").toDouble());
    if (num_taps % 2 == 0)
      {
        num_taps = num_taps + 1;
      }
    setParameter("# Taps", QString::number(num_taps));
    lambda1 = getParameter("Frequency 1 (Hz)").toDouble();
    lambda2 = getParameter("Frequency 2 (Hz)").toDouble();
    Kalpha = getParameter("Kaiser Alpha").toDouble();
    Calpha = getParameter("Chebyshev (dB)").toDouble();
    window_shape = window_t(windowShape->currentItem());
    filter_type = filter_t(filterType->currentItem());
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
    dt = RT::System::getInstance()->getPeriod() * 1e-9;
  default:
    break;
    }
}

// custom functions, as defined in the header file

void
FIRwindow::initParameters()
{
  dt = RT::System::getInstance()->getPeriod() * 1e-9; // s
  signalin.clear();
  assert(signalin.size() == 0);
  window_shape = HAMM;
  filter_type = LOWPASS;
  num_taps = 9;
  lambda1 = 0.1; // Hz / pi
  lambda2 = 0.6; // Hz / pi
  Calpha = 70; // dB
  Kalpha = 1.5;
  makeFilter();
  bookkeep();
}

void
FIRwindow::bookkeep()
{
  count = 0;
  systime = 0;
  signalin.set_capacity(2 * num_taps);
  convolution = new double[num_taps];
  for (int i = 0; i < 2 * num_taps; i++)
    { // pad with zeros
      signalin.push_back(0);
    }
  assert(signalin.full()); // check size and padding
  assert(signalin.size() == 2*num_taps);
  assert(signalin.capacity() == 2*num_taps);
}

void
FIRwindow::updateWindow(int index)
{
  if (index == 0)
    {
      window_shape = RECT;
      printf("Filter window now set to rectangular\n");
      makeFilter();
    }
  else if (index == 1)
    {
      window_shape = TRI;
      printf("Filter window now set to triangular\n");
      makeFilter();
    }
  else if (index == 2)
    {
      window_shape = HAMM;
      printf("Filter window now set to Hamming\n");
      makeFilter();
    }
  else if (index == 3)
    {
      window_shape = HANN;
      printf("Filter window now set to Hann\n");
      makeFilter();
    }
  else if (index == 4)
    {
      window_shape = CHEBY;
      printf("Filter window now set to Chebyshev\n");
      makeFilter();
    }
  else if (index == 5)
    {
      window_shape = KAISER;
      printf("Filter window now set to Kaiser\n");
      makeFilter();
    }
}

void
FIRwindow::updateFilterType(int index)
{
  if (index == 0)
    {
      filter_type = LOWPASS;
      printf("Filter type now set to LOWPASS\n");
      makeFilter();
    }
  else if (index == 1)
    {
      filter_type = HIGHPASS;
      printf("Filter type now set to HIGHPASS\n");
      makeFilter();
    }
  else if (index == 2)
    {
      filter_type = BANDPASS;
      printf("Filter type now set to BANDPASS\n");
      makeFilter();
    }
  else if (index == 3)
    {
      filter_type = BANDSTOP;
      printf("Filter type now set to BANDSTOP\n");
      makeFilter();
    }
}

void
FIRwindow::makeFilter()
{
  switch (window_shape)
    {
  case RECT: // rectangular
    disc_window = new RectangularWindow(num_taps);
    break;
  case TRI: // triangular
    disc_window = new TriangularWindow(num_taps, 1);
    break;
  case HAMM: // Hamming
    disc_window = new HammingWindow(num_taps);
    break;
  case HANN: // Hann
    disc_window = new HannWindow(num_taps, 1);
    break;
  case CHEBY: // Dolph-Chebyshev
    disc_window = new DolphChebyWindow(num_taps, Calpha);
    break;
  case KAISER:
    disc_window = new KaiserWindow(num_taps, Kalpha);
    break;
    } // end of switch on window_shape
  h1 = disc_window->GetDataWindow();
  filter_design = new FirIdealFilter(num_taps, lambda1, lambda2, filter_type);
  //	h2 = filter_design->GetCoefficients();
  filter_design->ApplyWindow(disc_window);
  h3 = filter_design->GetCoefficients();
  printf("\n      Windowed Filter\n");
  for (int i = 0; i < num_taps; i++)
    {
      printf("h[%i] = %f\n", i, h3[i]);
    }
  printf("\n");
}

void
FIRwindow::saveFIRData()
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
          //			stream.setPrintableData(true);
          switch (filter_type)
            {
          case LOWPASS:
            stream << QString("LOWPASS lambda1=") << (double) lambda1;
            break;
          case HIGHPASS:
            stream << QString("HIGHPASS lambda1=") << (double) lambda1;
            break;
          case BANDPASS:
            stream << QString("BANDPASS lambda1=") << (double) lambda1
                << " lambda2= " << (double) lambda2;
            break;
          case BANDSTOP:
            stream << QString("BANDSTOP lambda1=") << (double) lambda1
                << " lambda2= " << (double) lambda2;
            break;
            }
          stream << QString(" ");
          switch (window_shape)
            {
          case RECT: // rectangular
            stream << QString("RECT taps:") << num_taps << "\n";
            break;
          case TRI: // triangular
            stream << QString("TRI taps:") << num_taps << "\n";
            break;
          case HAMM: // Hamming
            stream << QString("HAMM taps:") << num_taps << "\n";
            break;
          case HANN: // Hann
            stream << QString("HANN taps:") << num_taps << "\n";
            break;
          case CHEBY: // Dolph-Chebyshev
            stream << QString("CHEBY taps:") << num_taps << " alpha:" << Calpha
                << "\n";
            break;
          case KAISER:
            stream << QString("KAISER taps:") << num_taps << " alpha:"
                << Kalpha << "\n";
            break;
            }
          printf("Filter impulse response:\n");
          for (int i = 0; i < num_taps; i++)
            {
              printf("%f\n", h3[i]);
              stream << QString("h[") << i << "] = " << (double) h3[i] << "\n";
            }
          dataFile.close();
          printf("File closed.\n");
        }
      else
        {
          QMessageBox::information(this, "FIR filter: Save filter parameters",
              "There was an error writing to this file. You can view\n"
                "the parameters in the terminal.\n");
        }
    }
}

bool
FIRwindow::OpenFile(QString FName)
{
  dataFile.setName(FName);
  if (dataFile.exists())
    {
      switch (QMessageBox::warning(this, "FIR filter", tr(
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
  //	stream.setPrintableData(false); // write binary
  printf("File opened: %s\n", FName.latin1());
  return true;
}

//create the GUI components
void
FIRwindow::createGUI(DefaultGUIModel::variable_t *var, int size)
{
  QBoxLayout *layout = new QHBoxLayout(this); // overall GUI layout

  // Left side GUI
  QBoxLayout *leftlayout = new QVBoxLayout();
  QPushButton *saveDataButton = new QPushButton("Save FIR Parameters", this);
  leftlayout->addWidget(saveDataButton);

  QObject::connect(saveDataButton, SIGNAL(clicked()), this, SLOT(saveFIRData()));
  QToolTip::add(saveDataButton,
      "Save filter parameters and coefficients to a file");

  QGridLayout *optionLayout = new QGridLayout(leftlayout, 2, 2);
  QLabel *windowLabel = new QLabel("Window Shape:", this);
  windowShape = new QComboBox(FALSE, this, "Window Shape");
  windowShape->insertItem("Rectangular");
  windowShape->insertItem("Triangular (Bartlett)");
  windowShape->insertItem("Hamming");
  windowShape->insertItem("Hann");
  windowShape->insertItem("Chebyshev");
  windowShape->insertItem("Kaiser");
  QToolTip::add(windowShape,
      "Choose a window to apply. For no window, choose Rectangular.");
  optionLayout->addWidget(windowLabel, 0, 0);
  optionLayout->addWidget(windowShape, 0, 1);
  QObject::connect(windowShape,SIGNAL(activated(int)), this, SLOT(updateWindow(int)));

  QLabel *filterLabel = new QLabel("Type of Filter:", this);
  filterType = new QComboBox(FALSE, this, "Filter Type");
  QToolTip::add(filterType, "A Type 1 FIR filter.");
  filterType->insertItem("Lowpass");
  filterType->insertItem("Highpass");
  filterType->insertItem("Bandpass");
  filterType->insertItem("Bandstop");
  optionLayout->addWidget(filterLabel, 1, 0);
  optionLayout->addWidget(filterType, 1, 1);
  QObject::connect(filterType,SIGNAL(activated(int)), this, SLOT(updateFilterType(int)));

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

  show();

}
