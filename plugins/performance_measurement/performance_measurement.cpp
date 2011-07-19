/*
 * Copyright (C) 2005 Boston University
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License as
 *  published by the Free Software Foundation; either version 2 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <qhbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qtimer.h>

#include <debug.h>
#include <main_window.h>
#include <performance_measurement.h>

PerformanceMeasurement::Panel::Panel(QWidget *parent)
    : QWidget(parent,0,Qt::WStyle_NormalBorder | Qt::WDestructiveClose), state(INIT1) {
    QHBox *hbox;
    QBoxLayout *layout = new QVBoxLayout(this);

    setCaption("Performance Monitor");

    hbox = new QHBox(this);
    layout->addWidget(hbox);
    (void)(new QLabel("Computation Time (us)",hbox))->setFixedWidth(175);
    durationEdit = new QLineEdit(hbox);

    hbox = new QHBox(this);
    layout->addWidget(hbox);
    (void)(new QLabel("Peak Computation Time (us)",hbox))->setFixedWidth(175);
    maxDurationEdit = new QLineEdit(hbox);

    hbox = new QHBox(this);
    layout->addWidget(hbox);
    (void)(new QLabel("Realtime Period  (us)",hbox))->setFixedWidth(175);
    timestepEdit = new QLineEdit(hbox);

    hbox = new QHBox(this);
    layout->addWidget(hbox);
    (void)(new QLabel("Peak Realtime Period (us)",hbox))->setFixedWidth(175);
    maxTimestepEdit = new QLineEdit(hbox);

    QPushButton *resetButton = new QPushButton("Reset",this);
    layout->addWidget(resetButton);
    QObject::connect(resetButton,SIGNAL(clicked(void)),this,SLOT(reset(void)));

    QTimer *timer = new QTimer(this);
    timer->start(500);
    QObject::connect(timer,SIGNAL(timeout(void)),this,SLOT(update(void)));

    setActive(true);
}

PerformanceMeasurement::Panel::~Panel(void) {
    Plugin::getInstance()->panel = 0;
}

void PerformanceMeasurement::Panel::read(void) {
    long long now = RT::OS::getTime();

    switch(state) {
      case EXEC:
          if(maxTimestep < now-lastRead)
              maxTimestep = now-lastRead;
          timestep = 0.9*timestep + 0.1*(now-lastRead);
          break;
      case INIT2:
          timestep = maxTimestep = now-lastRead;
          state = EXEC;
          break;
      case INIT1:
          state = INIT2;
    }

    lastRead = now;
}

void PerformanceMeasurement::Panel::write(void) {
    long long now = RT::OS::getTime();

    switch(state) {
      case EXEC:
          if(maxDuration < now-lastRead)
              maxDuration = now-lastRead;
          duration = 0.9*duration + 0.1*(now-lastRead);
          break;
      case INIT2:
          duration = maxDuration = now-lastRead;
          break;
      default:
          ERROR_MSG("PerformanceMeasurement::Panel::write : invalid state\n");
    }
}

void PerformanceMeasurement::Panel::reset(void) {
    state = INIT1;
}

void PerformanceMeasurement::Panel::update(void) {
    durationEdit->setText(QString::number(duration*1e-3));
    maxDurationEdit->setText(QString::number(maxDuration*1e-3));
    timestepEdit->setText(QString::number(timestep*1e-3));
    maxTimestepEdit->setText(QString::number(maxTimestep*1e-3));
}

extern "C" Plugin::Object *createRTXIPlugin(void *) {
    return PerformanceMeasurement::Plugin::getInstance();
}

PerformanceMeasurement::Plugin::Plugin(void)
    : panel(0) {
    menuID = MainWindow::getInstance()->createSystemMenuItem("Performance Measurement",this,SLOT(createPerformanceMeasurementPanel(void)));
    MainWindow::getInstance()->insertSystemMenuSeparator();
}

PerformanceMeasurement::Plugin::~Plugin(void) {
    MainWindow::getInstance()->removeSystemMenuItem(menuID);
    if(panel) delete panel;
    instance = 0;
    panel = 0;
}

void PerformanceMeasurement::Plugin::createPerformanceMeasurementPanel(void) {
    if(!panel) panel = new Panel(MainWindow::getInstance()->centralWidget());
    panel->show();
}

static Mutex mutex;
PerformanceMeasurement::Plugin *PerformanceMeasurement::Plugin::instance = 0;

PerformanceMeasurement::Plugin *PerformanceMeasurement::Plugin::getInstance(void) {
    if(instance)
        return instance;

    /*************************************************************************
     * Seems like alot of hoops to jump through, but allocation isn't        *
     *   thread-safe. So effort must be taken to ensure mutual exclusion.    *
     *************************************************************************/

    Mutex::Locker lock(&::mutex);
    if(!instance)
        instance = new Plugin();

    return instance;
}
