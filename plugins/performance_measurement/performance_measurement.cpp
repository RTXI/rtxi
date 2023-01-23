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

#include "debug.hpp"
#include "main_window.hpp"
#include "rt.hpp"
#include "module.hpp"
#include "event.hpp"

#include "performance_measurement.hpp"

PerformanceMeasurement::Panel::Panel(std::string name, MainWindow* main_window)
    : Modules::Panel(name, main_window)
{
  // Make Mdi
  QMdiSubWindow* subWindow = new QMdiSubWindow;
  subWindow->setWindowIcon(QIcon("/usr/local/share/rtxi/RTXI-widget-icon.png"));
  subWindow->setAttribute(Qt::WA_DeleteOnClose);
  subWindow->setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowCloseButtonHint
                            | Qt::WindowMinimizeButtonHint);
  this->getMainWindowPtr()->createMdi(subWindow);

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
      resetButton, SIGNAL(released(void)), this, SLOT(this->plugin_component->reset(void)));

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
  auto maxDuration = getValue<double>("maxDuration");
  auto maxTimestep = getValue<double>("maxTimestep");
  auto maxLatency = getValue<double>("maxLatency");
  auto period = getValue<double>("period");

  double duration = *(end_ticks) - *(start_ticks);
  double timestep = last_start_ticks - *(start_ticks);
  auto latency = timestep - period;

  switch (getValue<Modules::Variable::state_t>("state")) {
    case Modules::Variable::EXEC :
      if (maxTimestep < timestep){
        setValue("maxTimestep", timestep);
      }
      if (maxDuration < duration){
        setValue("maxDuration", duration);
      }
      if (maxLatency < latency){
        setValue("maxLatency", latency);
      }
      setValue("latency", latency);
      timestepStat.push(timestep);
      latencyStat.push(latency);
      break;
    case Modules::Variable::INIT :
      timestepStat.clear();
      latencyStat.clear();
      setValue("maxTimestep", timestep);
      setValue("maxLatency", timestep - period);
      timestepStat.push(timestep);
      latencyStat.push(latency);
      setValue("state", Modules::Variable::EXEC);
      break;
    case Modules::Variable::PERIOD :
      setValue("period", RT::OS::getPeriod());
      break;
    case Modules::Variable::MODIFY :
    case Modules::Variable::PAUSE :
    case Modules::Variable::UNPAUSE :
    case Modules::Variable::EXIT :
      break;
  }
  last_start_ticks = *start_ticks;
  setValue("jitter", latencyStat.std());
}

void PerformanceMeasurement::Panel::update()
{
  Modules::Plugin* hostplugin = this->getHostPlugin();
  const double nano2micro = 1e-3;
  auto duration = hostplugin->getComponentDoubleParameter("duration") * nano2micro;
  auto maxduration = hostplugin->getComponentDoubleParameter("maxDuration") * nano2micro;
  auto timestep = hostplugin->getComponentDoubleParameter("timestep") * nano2micro;
  auto maxtimestep = hostplugin->getComponentDoubleParameter("maxTimestep") * nano2micro;
  auto timestepjitter = hostplugin->getComponentDoubleParameter("jitter") * nano2micro;

  durationEdit->setText(QString::number(duration));
  maxDurationEdit->setText(QString::number(maxduration));
  timestepEdit->setText(QString::number(timestep));
  maxTimestepEdit->setText(QString::number(maxtimestep));
  timestepJitterEdit->setText(QString::number(timestepjitter));
  AppCpuPercentEdit->setText(QString::number(RT::OS::getCpuUsage()));
}

void PerformanceMeasurement::Panel::reset()
{
  this->getHostPlugin()->setComponentState("state", Modules::Variable::INIT);
}

// extern "C" Plugin::Object * createRTXIPlugin(void *)
// {
//     return PerformanceMeasurement::Plugin::getInstance();
// }

PerformanceMeasurement::Plugin::Plugin(Event::Manager* ev_manager, 
                                       MainWindow* mw) 
  : Modules::Plugin(ev_manager, mw, "RT Benchmarks")
{
  auto plugin_component = std::make_unique<PerformanceMeasurement::Component>(this);
  this->attachComponent(std::move(plugin_component));
  // MainWindow::getInstance()->createSystemMenuItem(
  //     "RT Benchmarks", this, SLOT(createPerformanceMeasurementPanel(void)));
}

std::unique_ptr<Modules::Plugin> createRTXIPlugin(Event::Manager* ev_manager, MainWindow* main_window)
{
  return std::move(std::make_unique<PerformanceMeasurement::Plugin>(ev_manager, main_window));
}
