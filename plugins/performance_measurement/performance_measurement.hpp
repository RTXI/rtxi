/*
	 The Real-Time eXperiment Interface (RTXI)
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

#ifndef PERFORMANCE_MEASUREMENT_H
#define PERFORMANCE_MEASUREMENT_H

#include <QWidget>
#include <QTextStream>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QtWidgets>

#include <plugin.h>
#include <rt.h>
#include <workspace.h>
#include <math/runningstat.h>

namespace PerformanceMeasurement
{
class Panel;
class Plugin : public QObject, public ::Plugin::Object
{

    Q_OBJECT
    friend class Panel;

public:
    static Plugin *getInstance(void);

public slots:
    void createPerformanceMeasurementPanel(void);
private:
    Plugin(void);
    ~Plugin(void);
    Plugin(const Plugin &) : QObject() {};
    Plugin & operator=(const Plugin &)
    {
        return *getInstance();
    };
    static Plugin *instance;
    Panel *panel;
}; // class Plugin

class Panel : public QWidget, public RT::Device, public Workspace::Instance
{
    Q_OBJECT

public:
    Panel(QWidget *);
    virtual
    ~Panel(void);

    /*!
     * Measures the real-time period of the system
     */
    void read(void);

    /*!
     * Measures the computation time for loaded modules
     */
    void write(void);

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

    enum
    {
        INIT1, INIT2, EXEC,
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

    QLineEdit *durationEdit;
    QLineEdit *timestepEdit;
    QLineEdit *maxDurationEdit;
    QLineEdit *maxTimestepEdit;
    QLineEdit *timestepJitterEdit;
    QLineEdit *AppCpuPercentEdit;
}; // class Panel
}; // namespace PerformanceMeasurement
#endif /* PERFORMANCE_MEASUREMENT_H */
