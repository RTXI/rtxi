/*
 Copyright (C) 2011 Georgia Institute of Technology, University of Utah, Will
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

#include <functional>

#include "debug.hpp"
#include "event.hpp"
#include "main_window.hpp"
#include "module.hpp"
#include "rt.hpp"

#include "performance_measurement.hpp"

PerformanceMeasurement::Panel::Panel(std::string name,
                                     MainWindow* main_window,
                                     Event::Manager* ev_manager)
    : Modules::Panel(name, main_window, ev_manager)
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
  QObject::connect(resetButton, SIGNAL(released()), this, SLOT(reset()));

  // Attach child widget to parent widget
  layout->addLayout(gridLayout);

  // Attach gridLayout to Widget
  setLayout(layout);
  setWindowTitle(tr(std::string(PerformanceMeasurement::MODULE_NAME).c_str()));

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

PerformanceMeasurement::Component::Component(Modules::Plugin* hplugin) : 
                                  Modules::Component(hplugin, 
                                                     std::string(MODULE_NAME),
                                                     std::vector<IO::channel_t>(),
                                                     PerformanceMeasurement::performance_measurement_vars)
{
  this->bind_execute_callback([&](){ this->callback(); });
}


void PerformanceMeasurement::Component::callback()
{
  auto maxDuration =
      getValue<double>(PerformanceMeasurement::PARAMETER::MAX_DURATION);
  auto maxTimestep =
      getValue<double>(PerformanceMeasurement::PARAMETER::MAX_TIMESTEP);
  auto maxLatency =
      getValue<double>(PerformanceMeasurement::PARAMETER::MAX_LATENCY);
  auto period = RT::OS::getPeriod();
  if (period < 0) {
    period = RT::OS::DEFAULT_PERIOD;
  }

  double duration = *(end_ticks) - *(start_ticks);
  double timestep = *(start_ticks)-last_start_ticks;
  auto latency = timestep - period;

  switch (getValue<Modules::Variable::state_t>(
      PerformanceMeasurement::PARAMETER::STATE))
  {
    case Modules::Variable::EXEC:
      if (maxTimestep < timestep) {
        setValue(PerformanceMeasurement::PARAMETER::MAX_TIMESTEP, timestep);
      }
      if (maxDuration < duration) {
        setValue(PerformanceMeasurement::PARAMETER::MAX_DURATION, duration);
      }
      if (maxLatency < latency) {
        setValue(PerformanceMeasurement::PARAMETER::MAX_LATENCY, latency);
      }
      setValue(PerformanceMeasurement::PARAMETER::LATENCY, latency);
      latencyStat.push(latency);
      setValue(PerformanceMeasurement::PARAMETER::TIMESTEP, timestep);
      setValue(PerformanceMeasurement::PARAMETER::DURATION, duration);
      break;
    case Modules::Variable::INIT:
      latencyStat.clear();
      latencyStat.push(0.0);
      setValue(PerformanceMeasurement::PARAMETER::MAX_TIMESTEP, 0.0);
      setValue(PerformanceMeasurement::PARAMETER::MAX_DURATION, 0.0);
      setValue(PerformanceMeasurement::PARAMETER::MAX_LATENCY, 0.0);
      setValue(PerformanceMeasurement::PARAMETER::STATE,
               Modules::Variable::EXEC);
      break;
    case Modules::Variable::PERIOD:
    case Modules::Variable::MODIFY:
    case Modules::Variable::PAUSE:
    case Modules::Variable::UNPAUSE:
    case Modules::Variable::EXIT:
      break;
  }
  last_start_ticks = *start_ticks;
  setValue(PerformanceMeasurement::PARAMETER::JITTER, latencyStat.std());
}

void PerformanceMeasurement::Component::setTickPointers(int64_t* s_ticks,
                                                        int64_t* e_ticks)
{
  this->start_ticks = s_ticks;
  this->end_ticks = e_ticks;
}

void PerformanceMeasurement::Panel::refresh()
{
  Modules::Plugin* hostplugin = this->getHostPlugin();
  const double nano2micro = 1e-3;
  auto duration = hostplugin->getComponentDoubleParameter(
                      PerformanceMeasurement::PARAMETER::DURATION)
      * nano2micro;
  auto maxduration = hostplugin->getComponentDoubleParameter(
                         PerformanceMeasurement::PARAMETER::MAX_DURATION)
      * nano2micro;
  auto timestep = hostplugin->getComponentDoubleParameter(
                      PerformanceMeasurement::PARAMETER::TIMESTEP)
      * nano2micro;
  auto maxtimestep = hostplugin->getComponentDoubleParameter(
                         PerformanceMeasurement::PARAMETER::MAX_TIMESTEP)
      * nano2micro;
  auto timestepjitter = hostplugin->getComponentDoubleParameter(
                            PerformanceMeasurement::PARAMETER::JITTER)
      * nano2micro;

  durationEdit->setText(QString::number(duration));
  maxDurationEdit->setText(QString::number(maxduration));
  timestepEdit->setText(QString::number(timestep));
  maxTimestepEdit->setText(QString::number(maxtimestep));
  timestepJitterEdit->setText(QString::number(timestepjitter));
  AppCpuPercentEdit->setText(QString::number(RT::OS::getCpuUsage()));
}

void PerformanceMeasurement::Panel::reset()
{
  this->getHostPlugin()->setComponentState(
      PerformanceMeasurement::PARAMETER::STATE, Modules::Variable::INIT);
}

PerformanceMeasurement::Plugin::Plugin(Event::Manager* ev_manager,
                                       MainWindow* mw)
    : Modules::Plugin(
        ev_manager, mw, std::string(PerformanceMeasurement::MODULE_NAME))
{
  auto plugin_component =
      std::make_unique<PerformanceMeasurement::Component>(this);
  this->attachComponent(std::move(plugin_component));
  std::vector<Event::Object*> events;
  Event::Object preperiod_event(Event::Type::RT_PREPERIOD_EVENT);
  Event::Object postperiod_event(Event::Type::RT_POSTPERIOD_EVENT);
  events.push_back(&preperiod_event);
  events.push_back(&postperiod_event);
  this->event_manager->postEvent(events);
  auto* performance_measurement_component =
      dynamic_cast<PerformanceMeasurement::Component*>(
          this->plugin_component.get());
  performance_measurement_component->setTickPointers(
      std::any_cast<int64_t*>(preperiod_event.getParam("pre-period")),
      std::any_cast<int64_t*>(postperiod_event.getParam("post-period")));
  
  Event::Object activation_event(Event::Type::RT_BLOCK_UNPAUSE_EVENT);
  activation_event.setParam("block", std::any(static_cast<IO::Block*>(this->plugin_component.get()))); 
  this->event_manager->postEvent(&activation_event);
}

std::unique_ptr<Modules::Plugin> PerformanceMeasurement::createRTXIPlugin(
    Event::Manager* ev_manager, MainWindow* main_window)
{
  return std::make_unique<PerformanceMeasurement::Plugin>(ev_manager,
                                                          main_window);
}

Modules::Panel* PerformanceMeasurement::createRTXIPanel(
    MainWindow* main_window, Event::Manager* ev_manager)
{
  return static_cast<Modules::Panel*>(new PerformanceMeasurement::Panel(
      std::string(PerformanceMeasurement::MODULE_NAME),
      main_window,
      ev_manager));
}

std::unique_ptr<Modules::Component> PerformanceMeasurement::createRTXIComponent(
    Modules::Plugin* host_plugin)
{
  return std::make_unique<PerformanceMeasurement::Component>(host_plugin);
}

Modules::FactoryMethods PerformanceMeasurement::getFactories()
{
  Modules::FactoryMethods fact;
  fact.createPanel = &PerformanceMeasurement::createRTXIPanel;
  fact.createComponent = &PerformanceMeasurement::createRTXIComponent;
  fact.createPlugin = &PerformanceMeasurement::createRTXIPlugin;
  return fact;
}
