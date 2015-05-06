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

#include <debug.h>
#include <main_window.h>
#include <performance_measurement.h>

static Workspace::variable_t vars[] = { 
	{ "Comp Time (ns)", "", Workspace::STATE, },
	{ "Peak Comp Time (ns)", "", Workspace::STATE, }, 
	{ "Real-time Period (ns)", "", Workspace::STATE, },
	{ "Peak RT Period (ns)", "", Workspace::STATE, },
	{ "RT Jitter (ns)", "", Workspace::STATE, },
};

static size_t num_vars = sizeof(vars) / sizeof(Workspace::variable_t); // Required variable (number of variables)

PerformanceMeasurement::Panel::Panel(QWidget *parent) : QWidget(parent),
	Workspace::Instance("Performance Measurement", vars, num_vars), state(INIT1), duration(0),
	lastRead(0), timestep(0), maxDuration(0), maxTimestep(0), jitter(0) {

		QWidget::setAttribute(Qt::WA_DeleteOnClose);

		// Make Mdi
		QMdiSubWindow *subWindow = new QMdiSubWindow;
	   subWindow->setWindowIcon(QIcon("/usr/local/lib/rtxi/RTXI-widget-icon.png"));
		subWindow->setAttribute(Qt::WA_DeleteOnClose);
		subWindow->setWindowFlags(Qt::CustomizeWindowHint);
		subWindow->setWindowFlags(Qt::WindowCloseButtonHint);
		subWindow->setFixedSize(310,200);
		MainWindow::getInstance()->createMdi(subWindow);

		// Create main layout
		QVBoxLayout *layout = new QVBoxLayout;
		QString suffix = QString("s)").prepend(QChar(0x3BC));

		// Create child widget and gridLayout
		QGridLayout *gridLayout = new QGridLayout;

		durationEdit = new QLineEdit(subWindow);
		durationEdit->setReadOnly(true);
		gridLayout->addWidget(new QLabel(tr("Computation Time (").append(suffix)), 1, 0);
		gridLayout->addWidget(durationEdit, 1, 1);

		maxDurationEdit = new QLineEdit(subWindow);
		maxDurationEdit->setReadOnly(true);
		gridLayout->addWidget(new QLabel(tr("Peak Computation Time (").append(suffix)), 2, 0);
		gridLayout->addWidget(maxDurationEdit, 2, 1);

		timestepEdit = new QLineEdit(subWindow);
		timestepEdit->setReadOnly(true);
		gridLayout->addWidget(new QLabel(tr("Real-time Period (").append(suffix)), 3, 0);
		gridLayout->addWidget(timestepEdit, 3, 1);

		maxTimestepEdit = new QLineEdit(subWindow);
		maxTimestepEdit->setReadOnly(true);
		gridLayout->addWidget(new QLabel(tr("Peak Real-time Period (").append(suffix)), 4, 0);
		gridLayout->addWidget(maxTimestepEdit, 4, 1);

		timestepJitterEdit = new QLineEdit(subWindow);
		timestepJitterEdit->setReadOnly(true);
		gridLayout->addWidget(new QLabel(tr("Real-time Jitter (").append(suffix)), 5, 0);
		gridLayout->addWidget(timestepJitterEdit, 5, 1);

		QPushButton *resetButton = new QPushButton("Reset", this);
		gridLayout->addWidget(resetButton, 6, 1);
		QObject::connect(resetButton,SIGNAL(released(void)),this,SLOT(reset(void)));

		// Attach child widget to parent widget
		layout->addLayout(gridLayout);

		// Attach gridLayout to Widget
		setLayout(layout);
		setWindowTitle(QString::number(getID()) + tr(" RT Benchmarks"));

		// Set layout to Mdi
		subWindow->setWidget(this);
		show();

		QTimer *timer = new QTimer(this);
		timer->start(1000);
		QObject::connect(timer,SIGNAL(timeout(void)),this,SLOT(update(void)));
		resetMaxTimer = new QTimer(this);
		QObject::connect(resetMaxTimer,SIGNAL(timeout(void)),this,SLOT(resetMaxTimeStep(void)));

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
			timestep = now - lastRead;
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
	jitter = timestepStat.std();
}

void PerformanceMeasurement::Panel::write(void) {
	long long now = RT::OS::getTime();

	switch (state) {
		case EXEC:
			if (maxDuration < now - lastRead)
			{
				maxDuration = now - lastRead;
				resetMaxTimer->start(10000);				
			}
			duration = now - lastRead;
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

void PerformanceMeasurement::Panel::resetMaxTimeStep(void) {
	maxTimestep = 0.0;
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
	MainWindow::getInstance()->createSystemMenuItem("RT Benchmarks",this,SLOT(createPerformanceMeasurementPanel(void)));
}

PerformanceMeasurement::Plugin::~Plugin(void) {
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
