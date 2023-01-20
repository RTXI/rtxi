/*
         The Real-Time eXperiment Interface (RTXI)
         Copyright (C) 2011 Georgia Institute of Technology, University of Utah,
   Weill Cornell Medical College

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

#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QObject>
#include <QTextStream>
#include <QtWidgets>

#include "math/runningstat.h"
#include "module.hpp"
#include "rt.hpp"

namespace PerformanceMeasurement
{

struct stats_info
{
  double duration;
  double timestep;
  double latency;
  double maxDuration;
  double maxTimestep;
  double maxLatency;
  double jitter;
  double period;
};

const std::vector<Modules::Variable::Info> performance_measurement_vars 
{
  {
    "state",
    "RT Benchmarks State",
    Modules::Variable::STATE,
    Modules::Variable::INIT
  },
  {
    "duration",
    "Average time in nanoseconds for Real-Time loop computations",
    Modules::Variable::DOUBLE_PARAMETER,
    0.0
  },
  {
    "timestep",
    "Average time in nanoseconds for Real-Time period",
    Modules::Variable::DOUBLE_PARAMETER,
    0.0
  },
  {
    "latency",
    "Average time in nanoseconds for latency between expected wakeup and period start",
    Modules::Variable::DOUBLE_PARAMETER,
    0.0
  },
  {
    "maxDuration",
    "Maximum duration stat recorded in nanoseconds",
    Modules::Variable::DOUBLE_PARAMETER,
    0.0
  },
  {
    "maxTimestep",
    "maximum real-time period recorded in nanoseconds",
    Modules::Variable::DOUBLE_PARAMETER,
    0.0
  },
  {
    "maxLatency",
    "Maximum latency stat recorded in nanoseconds",
    Modules::Variable::DOUBLE_PARAMETER,
    0.0
  },
  {
    "jitter",
    "",
    Modules::Variable::DOUBLE_PARAMETER,
    0.0
  }
};

class Plugin : public Modules::Plugin
{
public:
  Plugin(Event::Manager* ev_manager, MainWindow* mw);
};  // class Plugin

class Component : public Modules::Component
{
public: 
  Component(PerformanceMeasurement::Plugin* hplugin) : 
            Modules::Component(hplugin,
                               "RT Benchmarks",
                               std::vector<IO::channel_t>(),
                               PerformanceMeasurement::performance_measurement_vars){}
  void execute() override;

  double getPeriod();

  private:

  RunningStat timestepStat;
  RunningStat latencyStat;

  int64_t *start_ticks; // only accessed in rt
  int64_t *end_ticks; // only accessed in rt
  int64_t last_start_ticks;
};

class Panel : public Modules::Panel
{
  Q_OBJECT

public:
  Panel(std::string name, MainWindow* main_window);

public slots:
  /*!
   * Starts the statistics over
   */
  void reset(void);
  void resetMaxTimeStep(void);
  /*!
   * Updates the GUI with the latest values
   */
  void update(void);


private:
  QLineEdit* durationEdit;
  QLineEdit* timestepEdit;
  QLineEdit* maxDurationEdit;
  QLineEdit* maxTimestepEdit;
  QLineEdit* timestepJitterEdit;
  QLineEdit* AppCpuPercentEdit;
};  // class Panel

std::unique_ptr<Modules::Plugin> createRTXIPlugin();

};  // namespace PerformanceMeasurement
#endif /* PERFORMANCE_MEASUREMENT_H */
