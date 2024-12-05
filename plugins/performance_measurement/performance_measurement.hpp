/*
         The Real-Time eXperiment Interface (RTXI)
         Copyright (C) 2011 Georgia Institute of Technology, University of Utah,
   Will Cornell Medical College

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

#ifndef PERFORMANCE_MEASUREMENT_H
#define PERFORMANCE_MEASUREMENT_H

#include "math/runningstat.h"
#include "widgets.hpp"

namespace RT::OS
{
class Fifo;
}  // namespace RT::OS

class QLineEdit;

namespace PerformanceMeasurement
{

constexpr std::string_view MODULE_NAME = "RT Benchmarks";

inline std::vector<Widgets::Variable::Info> get_default_vars()
{
  return {{}};
}

inline std::vector<IO::channel_t> get_default_channels()
{
  return {
      {"Duration",
       "Real-Time measurement of time elapsed time (ns) for calculations each period",
       IO::OUTPUT},
      {"Time Step", "Real-Time measurement of system period", IO::OUTPUT},
      {"Latency", "Time between intended wake-up and real wake-up", IO::OUTPUT},
      {"Max Duration",
       "Maximum computation time measured since last reset",
       IO::OUTPUT},
      {"Max Time Step",
       "Maximum realtime period measured since last reset",
       IO::OUTPUT},
      {"Max Latency", "Maximum latency measured since last reset", IO::OUTPUT},
      {"Jitter",
       "Standard Deviation of the real-time period measured since last reset",
       IO::OUTPUT}};
}

struct performance_stats_t
{
  double duration = 0.0;
  double timestep = 0.0;
  double latency = 0.0;
  double max_duration = 0.0;
  double max_timestep = 0.0;
  double max_latency = 0.0;
  double jitter = 0.0;
};

class Plugin : public Widgets::Plugin
{
public:
  explicit Plugin(Event::Manager* ev_manager);
  performance_stats_t getSampleStat();

private:
  RT::OS::Fifo* component_fifo;
};  // class Plugin

class Component : public Widgets::Component
{
public:
  explicit Component(Widgets::Plugin* hplugin);

  void setTickPointers(int64_t* s_ticks, int64_t* e_ticks);
  void execute() override;
  RT::OS::Fifo* getFIfoPtr() { return this->fifo.get(); }

private:
  performance_stats_t stats;

  // RunningStat timestepStat;
  RunningStat latencyStat;

  int64_t* start_ticks = nullptr;  // only accessed in rt
  int64_t* end_ticks = nullptr;  // only accessed in rt
  int64_t last_start_ticks = 0;
  std::unique_ptr<RT::OS::Fifo> fifo;
};

class Panel : public Widgets::Panel
{
  Q_OBJECT

public:
  Panel(const std::string& mod_name,
        QMainWindow* mwindow,
        Event::Manager* ev_manager);

public slots:
  /*!
   * Starts the statistics over
   */
  void reset();
  // void resetMaxTimeStep();
  /*!
   * Updates the GUI with the latest values
   */
  void refresh() override;

private:
  QLineEdit* durationEdit;
  QLineEdit* timestepEdit;
  QLineEdit* maxDurationEdit;
  QLineEdit* maxTimestepEdit;
  QLineEdit* timestepJitterEdit;
  QLineEdit* AppCpuPercentEdit;
};  // class Panel

std::unique_ptr<Widgets::Plugin> createRTXIPlugin(Event::Manager* ev_manager);

Widgets::Panel* createRTXIPanel(QMainWindow* main_window,
                                Event::Manager* ev_manager);

std::unique_ptr<Widgets::Component> createRTXIComponent(
    Widgets::Plugin* host_plugin);

Widgets::FactoryMethods getFactories();
}  // namespace PerformanceMeasurement
#endif /* PERFORMANCE_MEASUREMENT_H */
