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

#include <qhbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qtimer.h>
#include <qtooltip.h>

#include <debug.h>
#include <main_window.h>
#include <performance_measurement.h>

PerformanceMeasurement::Panel::Panel(QWidget *parent) :
  QWidget(parent, 0, Qt::WStyle_NormalBorder | Qt::WDestructiveClose), state(
      INIT1)
{
  QHBox *hbox;
  QBoxLayout *layout = new QVBoxLayout(this);

  setCaption("Real-time Benchmarks");

  hbox = new QHBox(this);
  layout->addWidget(hbox);
  QChar mu = QChar(0x3BC);
  QString suffix = QString("s)");

  QString labeltext = "Computation Time (";
  labeltext.append(mu);
  labeltext.append(suffix);
  (void) (new QLabel(labeltext, hbox))->setFixedWidth(175);
  durationEdit = new QLineEdit(hbox);

  hbox = new QHBox(this);
  layout->addWidget(hbox);
  labeltext = "Peak Computation Time (";
  labeltext.append(mu);
  labeltext.append(suffix);
  (void) (new QLabel(labeltext, hbox))->setFixedWidth(175);
  maxDurationEdit = new QLineEdit(hbox);

  hbox = new QHBox(this);
  layout->addWidget(hbox);
  labeltext = "Real-time Period (";
  labeltext.append(mu);
  labeltext.append(suffix);
  (void) (new QLabel(labeltext, hbox))->setFixedWidth(175);
  timestepEdit = new QLineEdit(hbox);

  hbox = new QHBox(this);
  layout->addWidget(hbox);
  labeltext = "Peak Real-time Period (";
  labeltext.append(mu);
  labeltext.append(suffix);
  (void) (new QLabel(labeltext, hbox))->setFixedWidth(175);
  maxTimestepEdit = new QLineEdit(hbox);
  QToolTip::add(maxTimestepEdit, "The worst case time step");

  hbox = new QHBox(this);
  layout->addWidget(hbox);
  labeltext = "Real-time Jitter (";
  labeltext.append(mu);
  labeltext.append(suffix);
  (void) (new QLabel(labeltext, hbox))->setFixedWidth(175);
  timestepJitterEdit = new QLineEdit(hbox);
  QToolTip::add(timestepJitterEdit, "The variance in the real-time period");

  hbox = new QHBox(this);
  layout->addWidget(hbox);
  QPushButton *startButton = new QPushButton("Start Saving", hbox);
  QObject::connect(startButton,SIGNAL(clicked(void)),this,SLOT(startSave(void)));
  QPushButton *stopButton = new QPushButton("Stop Saving", hbox);
  QObject::connect(stopButton,SIGNAL(clicked(void)),this,SLOT(stopSave(void)));

  QPushButton *resetButton = new QPushButton("Reset", this);
  layout->addWidget(resetButton);
  QObject::connect(resetButton,SIGNAL(clicked(void)),this,SLOT(reset(void)));

  QTimer *timer = new QTimer(this);
  timer->start(500);
  QObject::connect(timer,SIGNAL(timeout(void)),this,SLOT(update(void)));

  setActive(true);
  saveStats = false;
}

PerformanceMeasurement::Panel::~Panel(void)
{
  Plugin::getInstance()->panel = 0;
}

void
PerformanceMeasurement::Panel::read(void)
{
  long long now = RT::OS::getTime();

  switch (state)
    {
  case EXEC:
    if (maxTimestep < now - lastRead)
      maxTimestep = now - lastRead;
    timestep = 0.9 * timestep + 0.1 * (now - lastRead);
    timestepStat.push(timestep);
    break;
  case INIT2:
    timestep = maxTimestep = now - lastRead;
    timestepStat.push(timestep);
    state = EXEC;
    break;
  case INIT1:
    state = INIT2;
    }
  if (saveStats)
    stream << (double) timestep << "\n";
  lastRead = now;
}

void
PerformanceMeasurement::Panel::write(void)
{
  long long now = RT::OS::getTime();

  switch (state)
    {
  case EXEC:
    if (maxDuration < now - lastRead)
      maxDuration = now - lastRead;
    duration = 0.9 * duration + 0.1 * (now - lastRead);
    break;
  case INIT2:
    duration = maxDuration = now - lastRead;
    break;
  default:
    ERROR_MSG("PerformanceMeasurement::Panel::write : invalid state\n");
    }
}

void
PerformanceMeasurement::Panel::reset(void)
{
  state = INIT1;
  timestepStat.clear();
}

void
PerformanceMeasurement::Panel::update(void)
{
  durationEdit->setText(QString::number(duration * 1e-3));
  maxDurationEdit->setText(QString::number(maxDuration * 1e-3));
  timestepEdit->setText(QString::number(timestep * 1e-3));
  maxTimestepEdit->setText(QString::number(maxTimestep * 1e-3));
  timestepJitterEdit->setText(QString::number(timestepStat.var() * 1e-3));
}

void
PerformanceMeasurement::Panel::startSave()
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
          reset();
          saveStats = true;
        }
      else
        {
          QMessageBox::information(this,
              "Real-time Benchmarks: Save real-time period",
              "There was an error writing to this file.\n");
          saveStats = false;
        }
    }
}

void
PerformanceMeasurement::Panel::stopSave()
{
  dataFile.close();
  saveStats = false;
}

bool
PerformanceMeasurement::Panel::OpenFile(QString FName)
{
  dataFile.setName(FName);
  if (dataFile.exists())
    {
      switch (QMessageBox::warning(this, "Real-time Benchmarks", tr(
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
  stream << QString("time step in nanoseconds");
  //	stream.setPrintableData(false); // write binary
  printf("File opened for saving real-time benchmarks: %s\n", FName.latin1());
  return true;
}

extern "C" Plugin::Object *
createRTXIPlugin(void *)
{
  return PerformanceMeasurement::Plugin::getInstance();
}

PerformanceMeasurement::Plugin::Plugin(void) :
  panel(0)
{
menuID = MainWindow::getInstance()->createSystemMenuItem("Real-time Benchmarks",this,SLOT(createPerformanceMeasurementPanel(void)));
}

PerformanceMeasurement::Plugin::~Plugin(void)
{
  MainWindow::getInstance()->removeSystemMenuItem(menuID);
  if (panel)
    delete panel;
  instance = 0;
  panel = 0;
}

void
PerformanceMeasurement::Plugin::createPerformanceMeasurementPanel(void)
{
  if (!panel)
    panel = new Panel(MainWindow::getInstance()->centralWidget());
  panel->show();
}

static Mutex mutex;
PerformanceMeasurement::Plugin *PerformanceMeasurement::Plugin::instance = 0;

PerformanceMeasurement::Plugin *
PerformanceMeasurement::Plugin::getInstance(void)
{
  if (instance)
    return instance;

  /*************************************************************************
   * Seems like alot of hoops to jump through, but allocation isn't        *
   *   thread-safe. So effort must be taken to ensure mutual exclusion.    *
   *************************************************************************/

  Mutex::Locker lock(&::mutex);
  if (!instance)
    instance = new Plugin();

  return instance;
}
