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

#include <siggen.h>
#include <math.h>
#include <default_gui_model.h>
#include <main_window.h>
#include <qgridview.h>
#include <qhbox.h>
#include <qhbuttongroup.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qmessagebox.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qtimer.h>
#include <qtooltip.h>
#include <qvalidator.h>
#include <qvbuttongroup.h>
#include <qvbox.h>
#include <qwhatsthis.h>

extern "C" Plugin::Object *
createRTXIPlugin(void)
{
  return new SigGen();
}

static SigGen::variable_t vars[] =
  {
    { "Signal Waveform", "Noise Waveform", DefaultGUIModel::OUTPUT, },
        { "Delay", "Delay", DefaultGUIModel::PARAMETER
            | DefaultGUIModel::DOUBLE, },
        { "Width", "Width", DefaultGUIModel::PARAMETER
            | DefaultGUIModel::DOUBLE, },
        { "Freq (Hz)", "Freq (Hz), also used as minimum ZAP frequency",
            DefaultGUIModel::PARAMETER | DefaultGUIModel::DOUBLE, },
        { "Amplitude", "Amplitude", DefaultGUIModel::PARAMETER
            | DefaultGUIModel::DOUBLE, },
        { "ZAP max Freq (Hz)", "Maximum ZAP frequency",
            DefaultGUIModel::PARAMETER | DefaultGUIModel::DOUBLE, },
        { "ZAP duration (s)", "ZAP duration (s)", DefaultGUIModel::PARAMETER
            | DefaultGUIModel::DOUBLE, },
        { "Time (s)", "Time (s)", DefaultGUIModel::STATE, }, };

static size_t num_vars = sizeof(vars) / sizeof(DefaultGUIModel::variable_t);

SigGen::SigGen(void) :
  DefaultGUIModel("Signal Generator", ::vars, ::num_vars)
{
  QWhatsThis::add(
      this,
      "<p><b>Signal Generator:</b></p><p>Generates noise of the type specified with the relevant parameters:<br><br>"
        "Sine Wave: frequency, amplitude<br>"
        "Monophasic Square Wave: delay, pulse width, pulse amplitude<br>"
        "Biphasic Square Wave: delay, pulse width, pulse amplitude<br>"
        "Sawtooth Wave: delay, triangle width, triangle peak amplitude<br>"
        "ZAP stimulus: initial frequency, maximum frequency, duration of ZAP<br><br>"
        "The ZAP stimulus has the duration specified. All other signals are continuous signals.</p>");
  printf("Starting SigGen Module:\n");
  initParameters();
  initStimulus();
  createGUI(vars, num_vars); // this is required to create the GUI
  update(INIT);
  refresh();
}

SigGen::~SigGen(void)
{
}

void
SigGen::execute(void)
{
  systime = count * dt; // time in seconds

  if (mode == SINE)
    {
      output(0) = sineWave.get();
    }
  else if (mode == MONOSQUARE)
    {
      output(0) = monoWave.get();
    }
  else if (mode == BISQUARE)
    {
      output(0) = biWave.get();
    }
  else if (mode == SAWTOOTH)
    {
      output(0) = sawWave.get();
    }
  else if (mode == ZAP)
    {
      output(0) = zapWave.getOne();
    }
  else
    {
      output(0) = 0;
    }
  count++; // increment time

}

void
SigGen::update(DefaultGUIModel::update_flags_t flag)
{
  switch (flag)
    {
  case INIT:
    setParameter("Freq (Hz)", QString::number(freq));
    setParameter("ZAP max Freq (Hz)", QString::number(freq2));
    setParameter("ZAP duration (s)", QString::number(ZAPduration));
    setParameter("Delay", QString::number(delay));
    setParameter("Width", QString::number(width));
    setParameter("Amplitude", QString::number(amp));
    setState("Time (s)", systime);
    updateMode(0);
    waveShape->setCurrentItem(mode);
    break;
  case MODIFY:
    delay = getParameter("Delay").toDouble();
    freq = getParameter("Freq (Hz)").toDouble();
    freq2 = getParameter("ZAP max Freq (Hz)").toDouble();
    ZAPduration = getParameter("ZAP duration (s)").toDouble();
    width = getParameter("Width").toDouble();
    amp = getParameter("Amplitude").toDouble();
    mode = mode_t(waveShape->currentItem());
    initStimulus();
    break;
  case PERIOD:
    dt = RT::System::getInstance()->getPeriod() * 1e-9; // time in seconds
    initStimulus();
  case PAUSE:
    output(0) = 0.0;
    zapWave.setIndex(0);
    break;
  case UNPAUSE:
    systime = 0;
    count = 0;
    break;
  default:
    break;
    }
}

void
SigGen::initParameters()
{
  freq = 1; // Hz
  amp = 1;
  width = 1; // s
  delay = 1; // s
  mode = SINE;
  freq2 = 20; // Hz
  ZAPduration = 10; //s

  dt = RT::System::getInstance()->getPeriod() * 1e-9; // s
  systime = 0;
  count = 0;
  output(0) = 0;

}

void
SigGen::initStimulus()
{

  switch (mode)
    {
  case SINE:
    sineWave.clear();
    sineWave.init(freq, amp, dt);
    break;
  case MONOSQUARE: // triangular
    monoWave.clear();
    monoWave.init(delay, width, amp, dt);
    break;
  case BISQUARE: // Hamming
    biWave.clear();
    biWave.init(delay, width, amp, dt);
    break;
  case SAWTOOTH: // Hann
    sawWave.clear();
    sawWave.init(delay, width, amp, dt);
    break;
  case ZAP: // Dolph-Chebyshev
    zapWave.clear();
    zapWave.init(freq, freq2, amp, ZAPduration, dt);
    break;
  default:
    break;
    }

}

void
SigGen::updateMode(int index)
{
  if (index == 0)
    {
      mode = SINE;
      printf("Signal generator now set to sine wave\n");
      update(MODIFY);
    }
  else if (index == 1)
    {
      mode = MONOSQUARE;
      printf("Signal generator now set to monophasic square wave\n");
      update(MODIFY);
    }
  else if (index == 2)
    {
      mode = BISQUARE;
      printf("Signal generator now set to biphasic square wave\n");
      update(MODIFY);
    }
  else if (index == 3)
    {
      mode = SAWTOOTH;
      printf("Signal generator now set to sawtooth wave\n");
      update(MODIFY);
    }
  else if (index == 4)
    {
      mode = ZAP;
      printf("Signal generator now set to a ZAP stimulus\n");
      update(MODIFY);
    }
}

void
SigGen::createGUI(DefaultGUIModel::variable_t *var, int size)
{

  QBoxLayout *layout = new QVBoxLayout(this);

  QVButtonGroup *modeBox = new QVButtonGroup("Signal Type", this);

  waveShape = new QComboBox(FALSE, modeBox, "Signal Type:");
  waveShape->insertItem("Sine Wave");
  waveShape->insertItem("Monophasic Square Wave");
  waveShape->insertItem("Biphasic Square Wave");
  waveShape->insertItem("Sawtooth Wave");
  waveShape->insertItem("ZAP Stimulus");
  QToolTip::add(waveShape, "Choose a signal to generate.");
  QObject::connect(waveShape,SIGNAL(activated(int)), this, SLOT(updateMode(int)));

  // add custom GUI components to layout above default_gui_model components
  layout->addWidget(modeBox);
  //layout->addWidget(waveShape);

  QScrollView *sv = new QScrollView(this);
  sv->setResizePolicy(QScrollView::AutoOneFit);
  layout->addWidget(sv);

  QWidget *viewport = new QWidget(sv->viewport());
  sv->addChild(viewport);
  QGridLayout *scrollLayout = new QGridLayout(viewport, 1, 2);

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
              param.edit->setPaletteForegroundColor(Qt::darkGray);
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

  QHBox *hbox1 = new QHBox(this);
  pauseButton = new QPushButton("Pause", hbox1);
  pauseButton->setToggleButton(true);
  QObject::connect(pauseButton,SIGNAL(toggled(bool)),this,SLOT(pause(bool)));
  QPushButton *modifyButton = new QPushButton("Modify", hbox1);
  QObject::connect(modifyButton,SIGNAL(clicked(void)),this,SLOT(modify(void)));
  QPushButton *unloadButton = new QPushButton("Unload", hbox1);
  QObject::connect(unloadButton,SIGNAL(clicked(void)),this,SLOT(exit(void)));
  layout->addWidget(hbox1);

  show();

}
