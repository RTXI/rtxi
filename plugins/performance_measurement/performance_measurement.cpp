/*
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

#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QTimer>
#include <QtGui>
#include <QToolTip>

#include <debug.h>
#include <main_window.h>
#include <performance_measurement.h>

static Workspace::variable_t vars[] = { 
	{ "Comp Time (ns)", "", Workspace::STATE, },
	{ "Peak Comp Time (ns)", "", Workspace::STATE, }, 
	{ "Real-time Period (ns)", "", Workspace::STATE, },
	{ "Peak RT Jitter (ns)", "", Workspace::STATE, },
	{ "RT Jitter (ns)", "", Workspace::STATE, },
};

static size_t num_vars = sizeof( vars ) / sizeof( Workspace::variable_t ); // Required variable (number of variables)

PerformanceMeasurement::Panel::Panel(QWidget *parent) : QWidget(parent),
	Workspace::Instance("Performance Measuremnt", vars, num_vars), state(INIT1), duration(0),
	lastRead(0), timestep(0), maxDuration(0), maxTimestep(0), jitter(0) {

		QWidget::setAttribute(Qt::WA_DeleteOnClose);

		// Create child widget and layout
		// Only need one for this module
		gridBox = new QGroupBox;
		QFormLayout *layout = new QFormLayout;
		QString suffix = QString("s)").prepend(QChar(0x3BC));

		durationEdit = new QLineEdit(gridBox);
		layout->addRow(new QLabel(tr("Computation Time (").append(suffix)), durationEdit);
		maxDurationEdit = new QLineEdit(gridBox);
		layout->addRow(new QLabel(tr("Peak Computation Time (").append(suffix)), maxDurationEdit);
		timestepEdit = new QLineEdit(gridBox);
		layout->addRow(new QLabel(tr("Real-time Period (").append(suffix)), timestepEdit);
		maxTimestepEdit = new QLineEdit(gridBox);
		layout->addRow(new QLabel(tr("Peak Real-time Period (").append(suffix)), maxTimestepEdit);
		timestepJitterEdit = new QLineEdit(gridBox);
		layout->addRow(new QLabel(tr("Real-time Jitter (").append(suffix)), timestepJitterEdit);

		QPushButton *resetButton = new QPushButton("Reset", this);
		layout->addWidget(resetButton);
		QObject::connect(resetButton,SIGNAL(clicked(void)),this,SLOT(reset(void)));

		setLayout(layout);
		setWindowTitle(tr("Real-time Benchmarks"));
		show();

		QTimer *timer = new QTimer(this);
		timer->start(500);
		QObject::connect(timer,SIGNAL(timeout(void)),this,SLOT(update(void)));

		// Connect states to workspace
		setData(Workspace::STATE, 0, &duration);
		setData(Workspace::STATE, 1, &maxDuration);
		setData(Workspace::STATE, 2, &timestep);
		setData(Workspace::STATE, 3, &maxTimestep);
		setData(Workspace::STATE, 4, &jitter);

		setActive(true);
		saveStats = false;
}

PerformanceMeasurement::Panel::~Panel(void) {
	Plugin::getInstance()->panel = 0;
}

void PerformanceMeasurement::Panel::read(void) {
	long long now = RT::OS::getTime();

	switch (state) {
		case EXEC:
			if (maxTimestep < now - lastRead)
				maxTimestep = now - lastRead;
			timestep = 0.9 * timestep + 0.1 * (now - lastRead);
			timestepStat.push(timestep);
			break;
		case INIT2:
			timestep = maxTimestep = now - lastRead;
			timestepStat.push(timestep);
			state = EXEC;
			break;
		case INIT1:
			state = INIT2;
	}
	lastRead = now;
	jitter = timestepStat.var();
}

void PerformanceMeasurement::Panel::write(void) {
	long long now = RT::OS::getTime();

	switch (state) {
		case EXEC:
			if (maxDuration < now - lastRead)
				maxDuration = now - lastRead;
			duration = 0.9 * duration + 0.1 * (now - lastRead);
			break;
		case INIT2:
			duration = maxDuration = now - lastRead;
			break;
		default:
			ERROR_MSG("PerformanceMeasurement::Panel::write : invalid state\n");
	}
}

void PerformanceMeasurement::Panel::reset(void) {
	state = INIT1;
	timestepStat.clear();
}

void PerformanceMeasurement::Panel::update(void) {
	durationEdit->setText(QString::number(duration * 1e-3));
	maxDurationEdit->setText(QString::number(maxDuration * 1e-3));
	timestepEdit->setText(QString::number(timestep * 1e-3));
	maxTimestepEdit->setText(QString::number(maxTimestep * 1e-3));  
	timestepJitterEdit->setText(QString::number(jitter * 1e-3));
}

extern "C" Plugin::Object * createRTXIPlugin(void *) {
	return PerformanceMeasurement::Plugin::getInstance();
}

PerformanceMeasurement::Plugin::Plugin(void) : panel(0) {
	MainWindow::getInstance()->createSystemMenuItem("Real-time Benchmarks",this,SLOT(createPerformanceMeasurementPanel(void)));
}

PerformanceMeasurement::Plugin::~Plugin(void) {
	MainWindow::getInstance()->removeSystemMenuItem(menuID);
	if (panel)
		delete panel;
	instance = 0;
	panel = 0;
}

void
PerformanceMeasurement::Plugin::createPerformanceMeasurementPanel(void) {
	if (!panel)
		panel = new Panel(MainWindow::getInstance()->centralWidget());
	panel->show();
}

static Mutex mutex;
PerformanceMeasurement::Plugin *PerformanceMeasurement::Plugin::instance = 0;

PerformanceMeasurement::Plugin * PerformanceMeasurement::Plugin::getInstance(void) {
	if (instance)
		return instance;

	/*************************************************************************
	 * Seems like alot of hoops to jump through, but allocation isn't        *
	 *   thread-safe. So effort must be taken to ensure mutual exclusion.    *
	 *************************************************************************/

	Mutex::Locker lock(&::mutex);
	if (!instance)
		instance = new Plugin();

	return instance;
}
