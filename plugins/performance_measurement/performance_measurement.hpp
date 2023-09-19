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

#include <QtWidgets>

#include "math/runningstat.h"
#include "module.hpp"
#include "rt.hpp"


namespace PerformanceMeasurement
{

constexpr std::string_view MODULE_NAME = "RT Benchmarks";

inline std::vector<Modules::Variable::Info> get_default_vars()
{ 
  return 
  {
    {
    }
  };
}

struct performance_stats_t{
  double duration=0.0;
  double timestep=0.0;
  double latency=0.0;
  double max_duration=0.0;
  double max_timestep=0.0;
  double max_latency=0.0;
  double jitter=0.0;
};

class Plugin : public Modules::Plugin
{
public:
  explicit Plugin(Event::Manager* ev_manager);
  performance_stats_t getSampleStat();
private:
  RT::OS::Fifo* component_fifo;
};  // class Plugin

class Component : public Modules::Component
{
public: 
  explicit Component(Modules::Plugin* hplugin); 

  void setTickPointers(int64_t* s_ticks, int64_t* e_ticks);
  void execute() override;
  RT::OS::Fifo* getFIfoPtr(){ return this->fifo.get(); }

private:
  performance_stats_t stats;

  //RunningStat timestepStat;
  RunningStat latencyStat;

  int64_t *start_ticks=nullptr; // only accessed in rt
  int64_t *end_ticks=nullptr; // only accessed in rt
  int64_t last_start_ticks=0;
  std::unique_ptr<RT::OS::Fifo> fifo;
};

class Panel : public Modules::Panel
{
  Q_OBJECT

public:
  Panel(const std::string& mod_name, QMainWindow* mwindow, Event::Manager* ev_manager);

public slots:
  /*!
   * Starts the statistics over
   */
  void reset();
  //void resetMaxTimeStep();
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

std::unique_ptr<Modules::Plugin> createRTXIPlugin(Event::Manager* ev_manager);

Modules::Panel* createRTXIPanel(QMainWindow* main_window, Event::Manager* ev_manager);

std::unique_ptr<Modules::Component> createRTXIComponent(Modules::Plugin* host_plugin);

Modules::FactoryMethods getFactories();
} // namespace PerformanceMeasurement
#endif /* PERFORMANCE_MEASUREMENT_H */
