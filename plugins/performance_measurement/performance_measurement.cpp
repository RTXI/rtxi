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

#include "performance_measurement.hpp"

#include "debug.hpp"
#include "event.hpp"
#include "rt.hpp"
#include "widgets.hpp"

PerformanceMeasurement::Panel::Panel(const std::string& mod_name,
                                     QMainWindow* mwindow,
                                     Event::Manager* ev_manager)
    : Widgets::Panel(mod_name, mwindow, ev_manager)
    , durationEdit(new QLineEdit(this))
    , timestepEdit(new QLineEdit(this))
    , maxDurationEdit(new QLineEdit(this))
    , maxTimestepEdit(new QLineEdit(this))
    , timestepJitterEdit(new QLineEdit(this))
    , AppCpuPercentEdit(new QLineEdit(this))
{
  // Create main layout
  // auto* box_layout = new QVBoxLayout;
  const QString suffix = QString("s)").prepend(QChar(0x3BC));

  // Create child widget and gridLayout
  auto* gridLayout = new QGridLayout;

  durationEdit->setReadOnly(true);
  gridLayout->addWidget(
      new QLabel(tr("Computation Time (").append(suffix)), 1, 0);
  gridLayout->addWidget(durationEdit, 1, 1);

  maxDurationEdit->setReadOnly(true);
  gridLayout->addWidget(
      new QLabel(tr("Peak Computation Time (").append(suffix)), 2, 0);
  gridLayout->addWidget(maxDurationEdit, 2, 1);

  timestepEdit->setReadOnly(true);
  gridLayout->addWidget(
      new QLabel(tr("Real-time Period (").append(suffix)), 3, 0);
  gridLayout->addWidget(timestepEdit, 3, 1);

  maxTimestepEdit->setReadOnly(true);
  gridLayout->addWidget(
      new QLabel(tr("Peak Real-time Period (").append(suffix)), 4, 0);
  gridLayout->addWidget(maxTimestepEdit, 4, 1);

  timestepJitterEdit->setReadOnly(true);
  gridLayout->addWidget(
      new QLabel(tr("Real-time Jitter (").append(suffix)), 5, 0);
  gridLayout->addWidget(timestepJitterEdit, 5, 1);

  AppCpuPercentEdit->setReadOnly(true);
  gridLayout->addWidget(new QLabel("RTXI App Cpu Usage(%)"), 6, 0);
  gridLayout->addWidget(AppCpuPercentEdit, 6, 1);

  auto* resetButton = new QPushButton("Reset", this);
  gridLayout->addWidget(resetButton, 7, 1);
  QObject::connect(resetButton,
                   &QPushButton::released,
                   this,
                   &PerformanceMeasurement::Panel::reset);

  // Attach child widget to parent widget
  // box_layout->addLayout(gridLayout);

  // Attach gridLayout to Widget
  setLayout(gridLayout);
  setWindowTitle(tr(std::string(PerformanceMeasurement::MODULE_NAME).c_str()));

  // Set layout to Mdi
  this->getMdiWindow()->setFixedSize(this->minimumSizeHint());
  auto* timer = new QTimer(this);
  timer->setInterval(1000);
  QObject::connect(
      timer, &QTimer::timeout, this, &PerformanceMeasurement::Panel::refresh);
  timer->start();
}

PerformanceMeasurement::Component::Component(Widgets::Plugin* hplugin)
    : Widgets::Component(hplugin,
                         std::string(MODULE_NAME),
                         std::vector<IO::channel_t>(),
                         PerformanceMeasurement::get_default_vars())
{
  if (RT::OS::getFifo(this->fifo,
                      10 * sizeof(PerformanceMeasurement::performance_stats_t))
      < 0)
  {
    ERROR_MSG(
        "PerformanceMeasurement::Component::Component : Unable to craate "
        "component fifo");
    this->setState(RT::State::PAUSE);
  }
}

void PerformanceMeasurement::Component::execute()
{
  auto period = RT::OS::getPeriod();
  if (period < 0) {
    period = RT::OS::DEFAULT_PERIOD;
  }

  stats.duration = static_cast<double>(*(end_ticks) - *(start_ticks));
  stats.timestep = static_cast<double>(*(start_ticks)-last_start_ticks);
  stats.latency = stats.timestep - static_cast<double>(period);
  stats.max_timestep = std::max(stats.max_timestep, stats.timestep);
  stats.max_duration = std::max(stats.max_duration, stats.duration);
  stats.max_latency = std::max(stats.max_latency, stats.latency);
  latencyStat.push(stats.latency);
  stats.jitter = latencyStat.std();

  switch (this->getState()) {
    case RT::State::EXEC:
      this->fifo->writeRT(&this->stats,
                          sizeof(PerformanceMeasurement::performance_stats_t));
      break;
    case RT::State::INIT:
      latencyStat.clear();
      latencyStat.push(0.0);
      this->stats = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
      this->setState(RT::State::EXEC);
      break;
    case RT::State::PERIOD:
      this->setState(RT::State::INIT);
      break;
    case RT::State::MODIFY:
    case RT::State::PAUSE:
    case RT::State::UNPAUSE:
    case RT::State::EXIT:
    case RT::State::UNDEFINED:
      break;
  }
  last_start_ticks = *start_ticks;
}

void PerformanceMeasurement::Component::setTickPointers(int64_t* s_ticks,
                                                        int64_t* e_ticks)
{
  this->start_ticks = s_ticks;
  this->end_ticks = e_ticks;
}

void PerformanceMeasurement::Panel::refresh()
{
  auto* hostplugin =
      dynamic_cast<PerformanceMeasurement::Plugin*>(this->getHostPlugin());
  const double nano2micro = 1e-3;
  const PerformanceMeasurement::performance_stats_t stats =
      hostplugin->getSampleStat();
  durationEdit->setText(QString::number(stats.duration * nano2micro));
  maxDurationEdit->setText(QString::number(stats.max_duration * nano2micro));
  timestepEdit->setText(QString::number(stats.timestep * nano2micro));
  maxTimestepEdit->setText(QString::number(stats.max_timestep * nano2micro));
  timestepJitterEdit->setText(QString::number(stats.jitter * nano2micro));
  AppCpuPercentEdit->setText(QString::number(RT::OS::getCpuUsage()));
}

void PerformanceMeasurement::Panel::reset()
{
  this->update_state(RT::State::INIT);
}

PerformanceMeasurement::Plugin::Plugin(Event::Manager* ev_manager)
    : Widgets::Plugin(ev_manager,
                      std::string(PerformanceMeasurement::MODULE_NAME))
{
  auto component = std::make_unique<PerformanceMeasurement::Component>(this);
  std::vector<Event::Object> events;
  events.emplace_back(Event::Type::RT_PREPERIOD_EVENT);
  events.emplace_back(Event::Type::RT_POSTPERIOD_EVENT);
  this->getEventManager()->postEvent(events);
  auto* performance_measurement_component =
      dynamic_cast<PerformanceMeasurement::Component*>(component.get());
  performance_measurement_component->setTickPointers(
      std::any_cast<int64_t*>(events[0].getParam("pre-period")),
      std::any_cast<int64_t*>(events[1].getParam("post-period")));

  this->component_fifo = component->getFIfoPtr();
  this->attachComponent(std::move(component));
}

PerformanceMeasurement::performance_stats_t
PerformanceMeasurement::Plugin::getSampleStat()
{
  PerformanceMeasurement::performance_stats_t stat;
  while (this->component_fifo->read(
             &stat, sizeof(PerformanceMeasurement::performance_stats_t))
         > 0)
  {
  };
  return stat;
}

std::unique_ptr<Widgets::Plugin> PerformanceMeasurement::createRTXIPlugin(
    Event::Manager* ev_manager)
{
  return std::make_unique<PerformanceMeasurement::Plugin>(ev_manager);
}

Widgets::Panel* PerformanceMeasurement::createRTXIPanel(
    QMainWindow* main_window, Event::Manager* ev_manager)
{
  return static_cast<Widgets::Panel*>(new PerformanceMeasurement::Panel(
      std::string(PerformanceMeasurement::MODULE_NAME),
      main_window,
      ev_manager));
}

std::unique_ptr<Widgets::Component> PerformanceMeasurement::createRTXIComponent(
    Widgets::Plugin* /*host_plugin*/)
{
  return std::make_unique<PerformanceMeasurement::Component>(nullptr);
}

Widgets::FactoryMethods PerformanceMeasurement::getFactories()
{
  Widgets::FactoryMethods fact;
  fact.createPanel = &PerformanceMeasurement::createRTXIPanel;
  fact.createComponent = &PerformanceMeasurement::createRTXIComponent;
  fact.createPlugin = &PerformanceMeasurement::createRTXIPlugin;
  return fact;
}
