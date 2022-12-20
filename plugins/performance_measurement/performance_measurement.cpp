/*
 Copyright (C) 2011 Georgia Institute of Technology, University of Utah, Weill
 Cornell Medical College

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

#include "performance_measurement.hpp"

#include "debug.hpp"
#include "main_window.hpp"
#include "rt.hpp"

PerformanceMeasurement::Panel::Panel(std::string name, QMainWindow* main_window)
    : Modules::Panel(name, main_window)
    , state(INIT1)
    , duration(0)
    , lastRead(0)
    , timestep(0)
    , maxDuration(0)
    , maxTimestep(0)
    , jitter(0)
{
  // Make Mdi
  QMdiSubWindow* subWindow = new QMdiSubWindow;
  subWindow->setWindowIcon(QIcon("/usr/local/share/rtxi/RTXI-widget-icon.png"));
  subWindow->setAttribute(Qt::WA_DeleteOnClose);
  subWindow->setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowCloseButtonHint
                            | Qt::WindowMinimizeButtonHint);
  this->main_window->createMdi(subWindow);

  // Create main layout
  QVBoxLayout* layout = new QVBoxLayout;
  QString suffix = QString("s)").prepend(QChar(0x3BC));

  // Create child widget and gridLayout
  QGridLayout* gridLayout = new QGridLayout;

  durationEdit = new QLineEdit(subWindow);
  durationEdit->setReadOnly(true);
  gridLayout->addWidget(
      new QLabel(tr("Computation Time (").append(suffix)), 1, 0);
  gridLayout->addWidget(durationEdit, 1, 1);

  maxDurationEdit = new QLineEdit(subWindow);
  maxDurationEdit->setReadOnly(true);
  gridLayout->addWidget(
      new QLabel(tr("Peak Computation Time (").append(suffix)), 2, 0);
  gridLayout->addWidget(maxDurationEdit, 2, 1);

  timestepEdit = new QLineEdit(subWindow);
  timestepEdit->setReadOnly(true);
  gridLayout->addWidget(
      new QLabel(tr("Real-time Period (").append(suffix)), 3, 0);
  gridLayout->addWidget(timestepEdit, 3, 1);

  maxTimestepEdit = new QLineEdit(subWindow);
  maxTimestepEdit->setReadOnly(true);
  gridLayout->addWidget(
      new QLabel(tr("Peak Real-time Period (").append(suffix)), 4, 0);
  gridLayout->addWidget(maxTimestepEdit, 4, 1);

  timestepJitterEdit = new QLineEdit(subWindow);
  timestepJitterEdit->setReadOnly(true);
  gridLayout->addWidget(
      new QLabel(tr("Real-time Jitter (").append(suffix)), 5, 0);
  gridLayout->addWidget(timestepJitterEdit, 5, 1);

  AppCpuPercentEdit = new QLineEdit(subWindow);
  AppCpuPercentEdit->setReadOnly(true);
  gridLayout->addWidget(new QLabel("RTXI App Cpu Usage(%)"), 6, 0);
  gridLayout->addWidget(AppCpuPercentEdit, 6, 1);

  QPushButton* resetButton = new QPushButton("Reset", this);
  gridLayout->addWidget(resetButton, 7, 1);
  QObject::connect(
      resetButton, SIGNAL(released(void)), this, SLOT(reset(void)));

  // Attach child widget to parent widget
  layout->addLayout(gridLayout);

  // Attach gridLayout to Widget
  setLayout(layout);
  setWindowTitle(tr(" RT Benchmarks"));

  // Set layout to Mdi
  subWindow->setWidget(this);
  subWindow->setFixedSize(subWindow->minimumSizeHint());
  show();

  QTimer* timer = new QTimer(this);
  timer->setTimerType(Qt::PreciseTimer);
  timer->start(1000);
  QObject::connect(timer, SIGNAL(timeout(void)), this, SLOT(update(void)));

  // // Connect states to workspace
  // setData(Modules::Variable::UINT_PARAMETER, 0ull, &duration);
  // setData(Modules::Variable::UINT_PARAMETER, 1ull, &maxDuration);
  // setData(Modules::Variable::UINT_PARAMETER, 2ull, &timestep);
  // setData(Modules::Variable::UINT_PARAMETER, 3ull, &maxTimestep);
  // setData(Modules::Variable::UINT_PARAMETER, 4ull, &jitter);
}

void PerformanceMeasurement::Component::execute()
{
  switch (state) {
    case EXEC:
      if (maxTimestep < now - lastRead)
        maxTimestep = now - lastRead;
      timestep = now - lastRead;
      latency = (now - lastRead) - period;
      timestepStat.push(timestep);
      latencyStat.push(latency);
      break;
    case INIT2:
      timestep = maxTimestep = now - lastRead;
      latency = maxLatency = (now - lastRead) - period;
      timestepStat.push(timestep);
      latencyStat.push(latency);
      state = EXEC;
      break;
    case INIT1:
      state = INIT2;
  }
  lastRead = now;
  jitter = latencyStat.std();
}

void PerformanceMeasurement::Panel::write(void)
{
  long long now = RT::OS::getTime();

  switch (state) {
    case EXEC:
      if (maxDuration < now - lastRead) {
        maxDuration = now - lastRead;
      }
      duration = now - lastRead;
      break;
    case INIT2:
      duration = maxDuration = now - lastRead;
      break;
    default:
      ERROR_MSG("PerformanceMeasurement::Panel::write : invalid state\n");
  }
}

void PerformanceMeasurement::Panel::reset(void)
{
  state = INIT1;
  timestepStat.clear();
  latencyStat.clear();
}

void PerformanceMeasurement::Panel::resetMaxTimeStep(void)
{
  maxTimestep = timestep;
}

void PerformanceMeasurement::Panel::update(void)
{
  durationEdit->setText(QString::number(duration * 1e-3));
  maxDurationEdit->setText(QString::number(maxDuration * 1e-3));
  timestepEdit->setText(QString::number(timestep * 1e-3));
  maxTimestepEdit->setText(QString::number(maxTimestep * 1e-3));
  timestepJitterEdit->setText(QString::number(jitter * 1e-3));
  AppCpuPercentEdit->setText(QString::number(RT::OS::getCpuUsage()));
}

// extern "C" Plugin::Object * createRTXIPlugin(void *)
// {
//     return PerformanceMeasurement::Plugin::getInstance();
// }

PerformanceMeasurement::Plugin::Plugin()
    : panel(0)
{
  MainWindow::getInstance()->createSystemMenuItem(
      "RT Benchmarks", this, SLOT(createPerformanceMeasurementPanel(void)));
}

PerformanceMeasurement::Component::Component(
    Modules::Plugin* hostPlugin,
    const std::string& name,
    std::vector<IO::channel_t> channels,
    std::vector<Modules::Variable::Info> variables)
    : Modules::Component(hostPlugin, name, channels, variables)
{
  this->
}