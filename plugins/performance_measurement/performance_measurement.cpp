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
#include "event.hpp"
#include "main_window.hpp"
#include "module.hpp"
#include "rt.hpp"

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
  auto period = RT::OS::getPeriod();
  if (period < 0) {
    period = RT::OS::DEFAULT_PERIOD;
  }

  double duration = *(end_ticks) - *(start_ticks);
  double timestep = *(start_ticks)-last_start_ticks;
  auto latency = timestep - period;

  switch (getValue<Modules::Variable::state_t>("state")) {
    case Modules::Variable::EXEC:
      if (maxTimestep < timestep) {
        setValue("maxTimestep", timestep);
      }
      if (maxDuration < duration) {
        setValue("maxDuration", duration);
      }
      if (maxLatency < latency) {
        setValue("maxLatency", latency);
      }
      setValue("latency", latency);
      latencyStat.push(latency);
      setValue("timestep", timestep);
      setValue("duration", duration);
      break;
    case Modules::Variable::INIT:
      latencyStat.clear();
      setValue("maxTimestep", 0.0);
      setValue("maxDuration", 0.0);
      setValue("maxLatency", 0.0);
      latencyStat.push(0.0);
      setValue("state", Modules::Variable::EXEC);
      break;
    case Modules::Variable::PERIOD:
    case Modules::Variable::MODIFY:
    case Modules::Variable::PAUSE:
    case Modules::Variable::UNPAUSE:
    case Modules::Variable::EXIT:
      break;
  }
  last_start_ticks = *start_ticks;
  setValue("jitter", latencyStat.std());
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
  auto duration =
      hostplugin->getComponentDoubleParameter("duration") * nano2micro;
  auto maxduration =
      hostplugin->getComponentDoubleParameter("maxDuration") * nano2micro;
  auto timestep =
      hostplugin->getComponentDoubleParameter("timestep") * nano2micro;
  auto maxtimestep =
      hostplugin->getComponentDoubleParameter("maxTimestep") * nano2micro;
  auto timestepjitter =
      hostplugin->getComponentDoubleParameter("jitter") * nano2micro;

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

PerformanceMeasurement::Plugin::Plugin(Event::Manager* ev_manager,
                                       MainWindow* mw)
    : Modules::Plugin(ev_manager, mw, "RT Benchmarks")
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
  this->setActive(true);
  // this->attachPanel(panel);
  //  MainWindow::getInstance()->createSystemMenuItem(
  //      "RT Benchmarks", this, SLOT(createPerformanceMeasurementPanel(void)));
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
      "RT Benchmarks", main_window, ev_manager));
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
