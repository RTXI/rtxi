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

#include <QtGui>
#include <QMdiArea>

#include <cmdline.h>
#include <debug.h>
#include <algorithm>
#include <settings.h>
#include <rtxi_config.h>
#include <stdlib.h>
#include <main_window.h>
#include <mutex.h>
#include <plugin.h>

MainWindow::MainWindow (void) : QMainWindow(NULL, Qt::Window) {

	// Make central RTXI parent widget
	mdiArea = new QMdiArea;
	setCentralWidget(mdiArea);

	/* Initialize Window Settings */
	setWindowTitle("RTXI - Real-time eXperimental Interface");
	setWindowIcon(QIcon("/usr/local/lib/rtxi/RTXI-icon.png"));

	/* Initialize Menus */
	createFileActions();
	createFileMenu();

	/* Initialize Module Menu */
	createModuleMenu();

	/* Initialize Utilities menu */
	createUtilMenu();

	/* Initialize System Menu */
	createSystemMenu();

	/* Initialize Windows Menu */
	createWindowsMenu();

	/* Initialize Help Menu */
	createHelpActions();
	createHelpMenu();

	updateUtilModules();
}

MainWindow::~MainWindow (void) {
}

QAction* MainWindow::insertModuleMenuSeparator (void) {
	return moduleMenu->addSeparator();
}

QAction* MainWindow::createFileMenuItem(const QString &text) {
	return fileMenu->addAction(text);
}

void MainWindow::clearFileMenu(void) {
	// Clear but add back default actions
	fileMenu->clear();
	fileMenu->addAction(load);
	fileMenu->addAction(save);
	fileMenu->addAction(reset);
	fileMenu->addSeparator();
	fileMenu->addAction(quit);
	fileMenu->addSeparator();
}

QAction* MainWindow::createModuleMenuItem(const QString &text) {
	return moduleMenu->addAction(text);
}

QAction* MainWindow::createModuleMenuItem(const QString &text, const QObject *receiver, const char *member) {
	return moduleMenu->addAction(text, receiver, member);
}

void MainWindow::setModuleMenuItemParameter (QAction *action, int parameter) {
	action->setData(parameter); //moduleMenu->setItemParameter (menuid, parameter);
}

void MainWindow::clearModuleMenu (void) {
	moduleMenu->clear();
}

void MainWindow::changeModuleMenuItem (QAction *action, QString text) {
	action->setText(text);
}

void MainWindow::removeModuleMenuItem (QAction *action) {
	QList<QAction *> actionList = moduleMenu->actions();
	if(!actionList.empty())
		moduleMenu->removeAction(action);
}

void MainWindow::removeModuleMenuItemAt (int id) {
	//moduleMenu->removeItemAt (id);
}

QAction* MainWindow::createUtilMenuItem(const QString &text, const QObject * receiver, const char *member) {
	return utilMenu->addAction (text, receiver, member);
}

void MainWindow::setUtilMenuItemParameter (QAction *action, int parameter) {
	action->setData(parameter);
	//utilMenu->setItemParameter (menuid, parameter);
}

void MainWindow::clearUtilMenu (void) {
	utilMenu->clear ();
}

void MainWindow::changeUtilMenuItem (int id, QString text) {
	//utilMenu->changeItem (id, text);
}

void MainWindow::removeUtilMenuItem (int id) {
	//utilMenu->removeItem(id);
}

void MainWindow::createFileMenu() {
	fileMenu = menuBar()->addMenu(tr("&File"));
	fileMenu->addAction(load);
	fileMenu->addAction(save);
	fileMenu->addAction(reset);
	fileMenu->addSeparator();
	fileMenu->addAction(quit);
	fileMenu->addSeparator();
	connect(fileMenu, SIGNAL(triggered(QAction*)), this, SLOT(fileMenuActivated(QAction*)));
}

void MainWindow::createModuleMenu() {
	moduleMenu = menuBar()->addMenu(tr("&Modules"));
	connect(moduleMenu, SIGNAL(triggered(QAction*)), this, SLOT(modulesMenuActivated(QAction*)));
}

void MainWindow::createUtilMenu() {
	utilMenu = menuBar()->addMenu(tr("&Utilities"));
	filtersSubMenu = utilMenu->addMenu(tr("&Filters"));
	utilitiesSubMenu = utilMenu->addMenu(tr("&Signal Generators"));
}

void MainWindow::createSystemMenu() {
	systemMenu = menuBar()->addMenu(tr("&System"));
}

void MainWindow::createWindowsMenu() {
	windowsMenu = menuBar()->addMenu(tr("&Windows"));
	connect(windowsMenu,SIGNAL(aboutToShow(void)),this,SLOT(windowsMenuAboutToShow(void)));
}

void MainWindow::createHelpMenu() {
	helpMenu = menuBar()->addMenu(tr("&Help"));
	helpMenu->addAction(artxi);
	helpMenu->addAction(aqt);
}

void MainWindow::createFileActions() {
	load = new QAction(tr("&Load Workspace"), this);
	load->setShortcuts(QKeySequence::Open);
	load->setStatusTip(tr("Load a saved workspace"));
	connect(load, SIGNAL(triggered()), this, SLOT(loadSettings()));

	save = new QAction(tr("&Save Workspace"), this);
	save->setShortcuts(QKeySequence::Save);
	save->setStatusTip(tr("Save current workspace"));
	connect(save, SIGNAL(triggered()), this, SLOT(saveSettings()));

	reset = new QAction(tr("&Reset Workspace"), this);
	reset->setStatusTip(tr("Reset to default RTXI workspace"));
	connect(reset, SIGNAL(triggered()), this, SLOT(resetSettings()));

	quit = new QAction(tr("&Quit"), this);
	quit->setShortcut(tr("Ctrl+Q"));
	quit->setStatusTip(tr("Quit RTXI"));
	connect(quit, SIGNAL(triggered()), qApp, SLOT(closeAllWindows()));
}

void MainWindow::createMdi(QMdiSubWindow *subWindow){
	mdiArea->addSubWindow(subWindow);
}

void MainWindow::createHelpActions() {
	artxi = new QAction(tr("About &RTXI"),this);
	connect(artxi, SIGNAL(triggered()), this, SLOT(about()));

	aqt = new QAction(tr("About &Qt"),this);
	connect(aqt, SIGNAL(triggered()), this, SLOT(aboutQt()));
}

void MainWindow::updateUtilModules(){
	QSettings userprefs;
	userprefs.setPath(QSettings::NativeFormat, QSettings::SystemScope, "/usr/local/share/rtxi/");
	userprefs.beginGroup("/utilFileList");
	QStringList entries = userprefs.childKeys();
	userprefs.endGroup();

	int numUtilFiles = entries.size();

	/*int i = 0;
		int menuID = utilMenu->insertItem("Model HH Neuron",this,SLOT(loadUtil(int)));
		setUtilMenuItemParameter(menuID, i);
		i++;
		menuID = utilMenu->insertItem("Synchronize Modules",this,SLOT(loadUtil(int)));
		setUtilMenuItemParameter(menuID, i);

		QPopupMenu *filterSubMenu = new QPopupMenu(this);
		i = 0;
		menuID = filterSubMenu->insertItem("FIR Filter (Window Method)",this,SLOT(loadFilter(int)));
		filterSubMenu->setItemParameter(menuID, i);
		i++;
		menuID = filterSubMenu->insertItem("IIR Filter",this,SLOT(loadFilter(int)));
		filterSubMenu->setItemParameter(menuID, i);

		utilMenu->insertItem(tr("&Filters"), filterSubMenu);

		QPopupMenu *signalSubMenu = new QPopupMenu(this);

		i = 0;
		menuID = signalSubMenu->insertItem("Signal Generator",this,SLOT(loadSignal(int)));
		signalSubMenu->setItemParameter(menuID, i);
		i++;
		menuID = signalSubMenu->insertItem("Noise Generator",this,SLOT(loadSignal(int)));
		signalSubMenu->setItemParameter(menuID, i);
		i++;
		menuID = signalSubMenu->insertItem("Wave Maker",this,SLOT(loadSignal(int)));
		signalSubMenu->setItemParameter(menuID, i);
		i++;
		menuID = signalSubMenu->insertItem("Mimic",this,SLOT(loadSignal(int)));
		signalSubMenu->setItemParameter(menuID, i);

		utilMenu->insertItem(tr("&Signals"), signalSubMenu);*/
}

QAction* MainWindow::createSystemMenuItem (const QString &text, const QObject *receiver, const char *member) {
	return systemMenu->addAction(text, receiver, member);
}

void MainWindow::about(void) {
	QMessageBox::about (this, "About RTXI", "Version " + QString(VERSION)
			+	"\n\nReleased under the GPLv3.\nSee www.rtxi.org for details.");
}

void MainWindow::aboutQt (void) {
	QMessageBox::aboutQt(this);
}

void MainWindow::loadSettings (void) {

	QSettings userprefs;
	userprefs.setPath (QSettings::NativeFormat, QSettings::SystemScope, "/usr/local/share/rtxi/");
	QString settingsDir = userprefs.value("/dirs/setfiles", getenv("HOME")).toString();

	QString filename = QFileDialog::getOpenFileName(this,
			tr("Load saved workspace"), "/home/", tr("Settings (*.set)"));

	if (QFile(filename).exists()) {
		systemMenu->clear();
		mdiArea->closeAllSubWindows();
		Settings::Manager::getInstance()->load(filename.toStdString());
	}
}

void MainWindow::saveSettings(void)
{
	QSettings userprefs;
	userprefs.setPath (QSettings::NativeFormat, QSettings::SystemScope, "/usr/local/share/rtxi/");
	QString settingsDir = userprefs.value("/dirs/setfiles", getenv("HOME")).toString();

	QString filename = QFileDialog::getSaveFileName(this,
			tr("Save current workspace"), "/home/", tr("Settings (*.set)"));

	if (!filename.isEmpty()) {
		if (!filename.endsWith(".set"))
			filename = filename+".set";
		if (QFileInfo (filename).exists() && QMessageBox::warning(this,
					"File Exists", "Do you wish to overwrite " + filename + "?",
					QMessageBox::Yes | QMessageBox::Default,
					QMessageBox::No | QMessageBox::Escape) != QMessageBox::Yes) {
			DEBUG_MSG ("MainWindow::saveSettings : canceled overwrite\n");
			return;
		}
		Settings::Manager::getInstance()->save(filename.toStdString());
	}
}

void MainWindow::resetSettings(void)
{
	systemMenu->clear();
	mdiArea->closeAllSubWindows();
	Settings::Manager::getInstance()->load("/etc/rtxi.conf");
}

/*void MainWindow::updateUtilModules () {
	QSettings userprefs;
	userprefs.setPath (QSettings::NativeFormat, QSettings::SystemScope, "/usr/local/share/rtxi/");

	QStringList entries = userprefs.entryList ("/utilFileList");
	int numUtilFiles = entries.size ();

	for (int i = 0; i < numUtilFiles; i++) {
	userprefs.removeEntry ("/utilFileList/util" + QString::number (i));
	userprefs.removeEntry ("/utilFileList/filter" + QString::number (i));
	userprefs.removeEntry ("/utilFileList/sig" + QString::number (i));
	}
	userprefs.writeEntry ("/utilFileList/util" + QString::number (0), "neuron.so");
	userprefs.writeEntry ("/utilFileList/util" + QString::number (1), "synch.so");

	userprefs.writeEntry ("/utilFileList/filter" + QString::number (0), "FIRwindow.so");
	userprefs.writeEntry ("/utilFileList/filter" + QString::number (1), "IIRfilter.so");

	userprefs.writeEntry ("/utilFileList/sig" + QString::number (0), "siggen.so");
	userprefs.writeEntry ("/utilFileList/sig" + QString::number (1), "noisegen.so");
	userprefs.writeEntry ("/utilFileList/sig" + QString::number (2), "wave_maker.so");
	userprefs.writeEntry ("/utilFileList/sig" + QString::number (3), "mimic.so");

	int i = 0;
	int menuID = utilMenu->insertItem("Model HH Neuron", this, SLOT (loadUtil (int)));
	setUtilMenuItemParameter(menuID, i);
	i++;
	menuID = utilMenu->insertItem("Synchronize Modules", this, SLOT (loadUtil (int)));
	setUtilMenuItemParameter(menuID, i);

	QMenu *filterSubMenu = new QMenu(this);
	i = 0;
	menuID = filterSubMenu->insertItem("FIR Filter (Window Method)", this, SLOT (loadFilter (int)));
	filterSubMenu->setItemParameter(menuID, i);
	i++;
	menuID = filterSubMenu->insertItem("IIR Filter", this, SLOT (loadFilter (int)));
	filterSubMenu->setItemParameter(menuID, i);

	utilMenu->insertItem(tr("&Filters"), filterSubMenu);

	QMenu *signalSubMenu = new QMenu(this);

	i = 0;
	menuID = signalSubMenu->insertItem("Signal Generator", this, SLOT (loadSignal (int)));
	signalSubMenu->setItemParameter(menuID, i);
	i++;
	menuID = signalSubMenu->insertItem("Noise Generator", this, SLOT (loadSignal (int)));
	signalSubMenu->setItemParameter(menuID, i);
	i++;
	menuID = signalSubMenu->insertItem("Wave Maker", this, SLOT (loadSignal (int)));
	signalSubMenu->setItemParameter(menuID, i);
	i++;
	menuID = signalSubMenu->insertItem("Mimic", this, SLOT (loadSignal (int)));
	signalSubMenu->setItemParameter(menuID, i);

	utilMenu->insertItem(tr ("&Signals"), signalSubMenu);
	}*/

void MainWindow::loadUtil(int i) {
	QSettings userprefs;
	userprefs.setPath (QSettings::NativeFormat, QSettings::SystemScope, "/usr/local/share/rtxi/");
	/*QString filename = userprefs.readEntry ("/utilFileList/util" + QString::number (i));
		QByteArray textData = filename.toLatin1();
		const char *text = textData.constData();
		Plugin::Manager::getInstance()->load(text);*/
}

void MainWindow::loadFilter(int i) {
	QSettings userprefs;
	userprefs.setPath (QSettings::NativeFormat, QSettings::SystemScope, "/usr/local/share/rtxi/");
	/*QString filename = userprefs.readEntry ("/utilFileList/filter"+ QString::number (i));
		QByteArray textData = filename.toLatin1();
		const char *text = textData.constData();
		Plugin::Manager::getInstance()->load(text);*/
}

void MainWindow::loadSignal(int i) {
	QSettings userprefs;
	userprefs.setPath (QSettings::NativeFormat, QSettings::SystemScope, "/usr/local/share/rtxi/");
	/*QString filename = userprefs.readEntry ("/utilFileList/sig" + QString::number (i));
		QByteArray textData = filename.toLatin1();
		const char *text = textData.constData();
		Plugin::Manager::getInstance()->load(text);*/
}

void MainWindow::windowsMenuAboutToShow(void) {

	// Clear previous entries
	windowsMenu->clear();

	// Add default options
	windowsMenu->addAction(tr("Cascade"),mdiArea,SLOT(cascadeSubWindows()));
	windowsMenu->addAction(tr("Tile"),mdiArea,SLOT(tileSubWindows()));
	windowsMenu->addSeparator();

	// Get list of open subwindows in Mdi Area
	subWindows = mdiArea->subWindowList();

	// Make sure it isn't empty
	if(subWindows.isEmpty())
		return;

	// Create windows list based off of what's open
	for(int i = 0; i < subWindows.size(); i++){
		QAction *item = new QAction(subWindows.at(i)->widget()->windowTitle(), this);
		windowsMenu->addAction(item);
	}
	connect(windowsMenu, SIGNAL(triggered(QAction*)), this, SLOT(windowsMenuActivated(QAction*)));
}

void MainWindow::windowsMenuActivated(QAction *id) {

	// Get list of open subwindows in Mdi Area
	subWindows = mdiArea->subWindowList();

	// Make sure it isn't empty
	if(subWindows.isEmpty())
		return;

	for(uint16_t i = 0; i < subWindows.size(); i++)
		if(subWindows.at(i)->widget()->windowTitle() == id->text())
			mdiArea->setActiveSubWindow(subWindows.at(i));
}

void MainWindow::modulesMenuActivated(QAction *id) {
	// Annoying but the best way to do it is to tie an action to the entire menu
	// so we have to tell it to ignore the first two modules
	if(id->text().contains("Load Plugin") ||
			id->text().contains("Load DYNAMO Model"))
		return;

	// Have to trim the first three characters before loading
	// or else parser will include qstring formatting
	Plugin::Manager::getInstance()->load(id->text().remove(0,3));
}

void MainWindow::fileMenuActivated(QAction *id) {
	// Annoying but the best way to do it is to tie an action to the entire menu
	// so we have to tell it to ignore the first three items
	if(id->text().contains("Load Workspace") ||
			id->text().contains("Save Workspace") ||
			id->text().contains("Reset Workspace") ||
			id->text().contains("Quit"))
		return;

	// Have to trim the first three characters before loading
	// or else parser will include qstring formatting
	systemMenu->clear();
	mdiArea->closeAllSubWindows();
	Settings::Manager::getInstance()->load(id->text().remove(0,3).toStdString());
}

static Mutex mutex;
MainWindow * MainWindow::instance = 0;

MainWindow * MainWindow::getInstance (void) {
	if (instance)
		return instance;

	/*************************************************************************
	 * Seems like alot of hoops to jump through, but static allocation isn't *
	 *   thread-safe. So effort must be taken to ensure mutual exclusion.    *
	 *************************************************************************/

	Mutex::Locker lock (&::mutex);
	if (!instance) {
		static MainWindow mainwindow;
		instance = &mainwindow;
	}
	return instance;
}
