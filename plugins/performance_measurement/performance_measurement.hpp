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
class Plugin : public Modules::Plugin
{
public:
  Plugin();
  ~Plugin();

private:
};  // class Plugin

class Component : public Modules::Component
{
public:
  Component(Modules::Plugin* hostPlugin,
            const std::string& name,
            std::vector<IO::channel_t> channels,
            std::vector<Modules::Variable::Info> variables) 
        
  virtual void execute() override;

  private:
  enum
  {
    INIT1,
    INIT2,
    EXEC,
  } state;

  double duration;
  double lastRead;
  double timestep;
  double latency;
  double maxDuration;
  double maxTimestep;
  double maxLatency;
  double jitter;

  RunningStat timestepStat;
  RunningStat latencyStat;
};

class Panel : public Modules::Panel
{
  Q_OBJECT

public:
  Panel(std::string name, QMainWindow* main_window);

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



  QLineEdit* durationEdit;
  QLineEdit* timestepEdit;
  QLineEdit* maxDurationEdit;
  QLineEdit* maxTimestepEdit;
  QLineEdit* timestepJitterEdit;
  QLineEdit* AppCpuPercentEdit;
};  // class Panel
};  // namespace PerformanceMeasurement
#endif /* PERFORMANCE_MEASUREMENT_H */
