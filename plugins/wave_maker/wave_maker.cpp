/*
 Copyright (C) 2011 Georgia Institute of Technology, University of Utah, Weill Cornell Medical College

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

#include <wave_maker.h>
#include <qfileinfo.h>
#include <qtextstream.h>
#include <math.h>
#include <time.h>
#include <qdatetime.h>
#include <qfile.h>
#include <qcombobox.h>
#include <main_window.h>
#include <qgridview.h>
#include <qhbox.h>
#include <qhbuttongroup.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qtimer.h>
#include <qtooltip.h>
#include <qvalidator.h>
#include <qvbox.h>
#include <qwhatsthis.h>

extern "C" Plugin::Object *
createRTXIPlugin(void)
{
  return new WaveMaker(); // Change the name of the plug-in here
}

static DefaultGUIModel::variable_t vars[] =
  {
    { "Output", "Signal from File", DefaultGUIModel::OUTPUT, },
    { "Loops", "Number of Times to Loop Data From File",
        DefaultGUIModel::PARAMETER | DefaultGUIModel::UINTEGER, },
    { "Length (s)", "Length of Trial is Computed From the Real-Time Period",
        DefaultGUIModel::STATE, },
    { "Gain", "Factor to Amplify the Signal", DefaultGUIModel::PARAMETER
        | DefaultGUIModel::DOUBLE, },
    { "File Name", "ASCII Input File", DefaultGUIModel::COMMENT, }, };

static size_t num_vars = sizeof(vars) / sizeof(DefaultGUIModel::variable_t);

WaveMaker::WaveMaker(void) :
  DefaultGUIModel("Wave Maker", ::vars, ::num_vars)
{
  QWhatsThis::add(
      this,
      "<p><b>Wave Maker:</b><br>This module reads a single value from an ascii file and outputs it as a signal.</p>");

  initParameters();
  createGUI(vars, num_vars);
  update( INIT);
  refresh();
}

WaveMaker::~WaveMaker(void)
{
}

void
WaveMaker::execute(void)
{
  if ((nloops && loop >= nloops) || !wave.size())
    {
      return;
    }

  output(0) = wave[idx++] * gain;

  if (idx >= wave.size())
    {
      idx = 0;
      if (nloops)
        ++loop;
    }
}

void
WaveMaker::createGUI(DefaultGUIModel::variable_t *var, int size)
{
  QBoxLayout *layout = new QHBoxLayout(this); // overall GUI layout

  // Left side GUI
  QBoxLayout *leftlayout = new QVBoxLayout();

  QHButtonGroup *fileBox = new QHButtonGroup("File:", this);
  QPushButton *loadBttn = new QPushButton("Load File", fileBox);
  QPushButton *previewBttn = new QPushButton("Preview File", fileBox);
  QObject::connect(loadBttn, SIGNAL(clicked()), this, SLOT(loadFile()));
  QObject::connect(previewBttn, SIGNAL(clicked()), this, SLOT(previewFile()));

  QHBox *utilityBox = new QHBox(this);
  pauseButton = new QPushButton("Pause", utilityBox);
  pauseButton->setToggleButton(true);
  QObject::connect(pauseButton, SIGNAL(toggled(bool)), this, SLOT(pause(bool)));
  QObject::connect(pauseButton, SIGNAL(toggled(bool)), loadBttn, SLOT(setEnabled(bool)));
  QPushButton *modifyButton = new QPushButton("Modify", utilityBox);
  QObject::connect(modifyButton, SIGNAL(clicked(void)), this, SLOT(modify(void)));
  QPushButton *unloadButton = new QPushButton("Unload", utilityBox);
  QObject::connect(unloadButton, SIGNAL(clicked(void)), this, SLOT(exit(void)));
  QObject::connect(pauseButton, SIGNAL(toggled(bool)), modifyButton, SLOT(setEnabled(bool)));
  QToolTip::add(pauseButton, "Start/Stop Plug-in");
  QToolTip::add(modifyButton, "Commit Changes to Parameter Values");
  QToolTip::add(unloadButton, "Close Plug-in");

  // Add custom left side GUI components to layout above default_gui_model components
  leftlayout->addWidget(fileBox);

  QScrollView *sv = new QScrollView(this);
  sv->setResizePolicy(QScrollView::AutoOneFit);
  leftlayout->addWidget(sv);

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

  // add custom components to layout below default_gui_model components
  leftlayout->addWidget(utilityBox);
  // Add left and right side layouts to the overall layout
  layout->addLayout(leftlayout);
  //	layout->setResizeMode(QLayout::Fixed);

  // set GUI refresh rate
  QTimer *timer = new QTimer(this);
  timer->start(1000);
  QObject::connect(timer, SIGNAL(timeout(void)), this, SLOT(refresh(void)));
  show();

}

void
WaveMaker::update(DefaultGUIModel::update_flags_t flag)
{
  switch (flag)
    {
  case INIT:
    setParameter("Loops", QString::number(nloops));
    setParameter("Gain", QString::number(gain));
    setComment("File Name", filename);
    setState("Length (s)", length);
    break;
  case MODIFY:
    nloops = getParameter("Loops").toUInt();
    gain = getParameter("Gain").toDouble();
    filename = getComment("File Name");
    break;
  case PAUSE:
    output(0) = 0; // stop command in case pause occurs in the middle of command
    idx = 0;
    loop = 0;
    gain = 0;
    printf("Protocol paused.\n");
    break;
  case UNPAUSE:
    printf("Protocol started.\n");
    break;
  case PERIOD:
    dt = RT::System::getInstance()->getPeriod() * 1e-9;
    loadFile(filename);
  default:
    break;
    }

}

void
WaveMaker::initParameters()
{
  dt = RT::System::getInstance()->getPeriod() * 1e-9; // s
  gain = 1;
  filename = "No file loaded.";
  idx = 0;
  loop = 0;
  nloops = 1;
  length = 0;
}

void
WaveMaker::loadFile()
{
  QFileDialog* fd = new QFileDialog(this, "Wave Maker Input File", TRUE);
  fd->setMode(QFileDialog::AnyFile);
  fd->setViewMode(QFileDialog::Detail);
  QString fileName;
  if (fd->exec() == QDialog::Accepted)
    {
      fileName = fd->selectedFile();
      printf("Loading new file: %s\n", fileName.latin1());
      setComment("File Name", fileName);
      wave.clear();
      QFile file(fileName);
      if (file.open(IO_ReadOnly))
        {
          QTextStream stream(&file);
          double value;
          while (!stream.atEnd())
            {
              stream >> value;
              wave.push_back(value);
            }
          filename = fileName;
        }
      length = wave.size() * dt;
      setState("Length (s)", length); // initialized in s, display in s
    }
  else
    {
      setComment("File Name", "No file loaded.");
    }
}

void
WaveMaker::loadFile(QString fileName)
{
  if (fileName == "No file loaded.")
    {
      printf("File not loaded.\n");
      return;
    }
  else
    {
      printf("Loading new file: %s\n", fileName.latin1());
      wave.clear();
      QFile file(fileName);
      if (file.open(IO_ReadOnly))
        {
          QTextStream stream(&file);
          double value;
          while (!stream.atEnd())
            {
              stream >> value;
              wave.push_back(value);
            }
        }
      length = wave.size() * dt;
      setState("Length (s)", length); // initialized in s, display in s
    }
}

void
WaveMaker::previewFile()
{
  double* time = new double[static_cast<int> (wave.size())];
  double* yData = new double[static_cast<int> (wave.size())];
  for (int i = 0; i < wave.size(); i++)
    {
      time[i] = dt * i;
      yData[i] = wave[i];
    }
  PlotDialog *preview = new PlotDialog(this, "Wave Maker Waveform", time, yData,
      wave.size());

  preview->show();

}

