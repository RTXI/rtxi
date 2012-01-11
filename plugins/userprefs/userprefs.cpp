/*
 * Copyright (C) 2011 Georgia Institute of Technology
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

/*
 * The settings are only written when the panel is closed and the QSettings
 * is destructed. So all Reset, Apply, and Cancel buttons all close the panel.
 */

#include <qhbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qfiledialog.h>
#include <debug.h>
#include <main_window.h>
#include <userprefs.h>

UserPrefs::Panel::Panel(QWidget *parent) :
QWidget(parent, 0, Qt::WStyle_NormalBorder | Qt::WDestructiveClose) {
	QHBox *hbox;
	QBoxLayout *layout = new QVBoxLayout(this);

	setCaption("RTXI User Preferences");

	// load QSettings
	QSettings userprefs;
	userprefs.setPath("RTXI.org", "RTXI", QSettings::User);

	// create GUI elements and set text

	hbox = new QHBox(this);
	layout->addWidget(hbox);
	(void) (new QLabel("Default settings directory:", hbox))->setFixedWidth(250);
	settingsDirEdit = new QLineEdit(hbox);
	settingsDirEdit->setFixedWidth(200);
	settingsDirEdit->setText(userprefs.readEntry("/dirs/setfiles", getenv(
			"HOME")));
	QPushButton *chooseSettingsDirButton = new QPushButton("Browse", hbox);
	QObject::connect(chooseSettingsDirButton,SIGNAL(clicked(void)),this,SLOT(chooseSettingsDir(void)));

	hbox = new QHBox(this);
	layout->addWidget(hbox);
	(void) (new QLabel("Default DYNAMO models directory:", hbox))->setFixedWidth(
			250);
	dynamoDirEdit = new QLineEdit(hbox);
	dynamoDirEdit->setText(userprefs.readEntry("/dirs/dynamomodels", getenv(
			"HOME")));
	QPushButton *chooseDynamoDirButton = new QPushButton("Browse", hbox);
	QObject::connect(chooseDynamoDirButton,SIGNAL(clicked(void)),this,SLOT(chooseDynamoDir(void)));

	hbox = new QHBox(this);
	layout->addWidget(hbox);
	(void) (new QLabel("Default HDF5 data directory:", hbox))->setFixedWidth(
			250);
	dataDirEdit = new QLineEdit(hbox);
	dataDirEdit->setText(userprefs.readEntry("/dirs/data", getenv("HOME")));
	QPushButton *chooseDataDirButton = new QPushButton("Browse", hbox);
	QObject::connect(chooseDataDirButton,SIGNAL(clicked(void)),this,SLOT(chooseDataDir(void)));

	hbox = new QHBox(this);

	QPushButton *resetButton = new QPushButton("Reset and Close", hbox);
	QObject::connect(resetButton,SIGNAL(clicked(void)),this,SLOT(reset(void)));

	QPushButton *applyButton = new QPushButton("Save and Close", hbox);
	QObject::connect(applyButton,SIGNAL(clicked(void)),this,SLOT(apply(void)));

	QPushButton *cancelButton = new QPushButton("Close without Saving", hbox);
	QObject::connect(cancelButton,SIGNAL(clicked(void)),this,SLOT(cancel(void)));

	layout->addWidget(hbox);

	show();

	//setActive(true);
}

UserPrefs::Panel::~Panel(void) {
	Prefs::getInstance()->panel = 0;
}

void UserPrefs::Panel::reset(void) {
	settingsDirEdit->setText(getenv("HOME"));
	dynamoDirEdit->setText(getenv("HOME"));
	dataDirEdit->setText(getenv("HOME"));
	apply();
}

void UserPrefs::Panel::apply(void) {
	userprefs.writeEntry("/dirs/setfiles", settingsDirEdit->text());
	userprefs.writeEntry("/dirs/dynamomodels", dynamoDirEdit->text());
	userprefs.writeEntry("/dirs/data", dataDirEdit->text());
	this->close();
}

void UserPrefs::Panel::cancel(void) {
	this->close();
}

void UserPrefs::Panel::chooseSettingsDir(void) {
	QString dir_name = QFileDialog::getExistingDirectory(userprefs.readEntry(
			"/dirs/setfiles", getenv("HOME")), MainWindow::getInstance(),
			"get existing directory",
			"Choose a default directory for settings files", TRUE);
	settingsDirEdit->setText(dir_name);
}

void UserPrefs::Panel::chooseDynamoDir(void) {
	QString dir_name = QFileDialog::getExistingDirectory(userprefs.readEntry(
			"/dirs/setfiles", getenv("HOME")), MainWindow::getInstance(),
			"get existing directory",
			"Choose a default directory for DYNAMO models", TRUE);
	dynamoDirEdit->setText(dir_name);
}

void UserPrefs::Panel::chooseDataDir(void) {
	QString dir_name = QFileDialog::getExistingDirectory(userprefs.readEntry(
			"/dirs/data", getenv("HOME")), MainWindow::getInstance(),
			"get existing directory",
			"Choose a default directory for HDF5 data files", TRUE);
	dataDirEdit->setText(dir_name);
}

extern "C" Plugin::Object *createRTXIPlugin(void *) {
	return UserPrefs::Prefs::getInstance();
}

UserPrefs::Prefs::Prefs(void) :
	panel(0) {
menuID = MainWindow::getInstance()->createSystemMenuItem("Preferences",this,SLOT(createPrefsPanel(void)));
}

UserPrefs::Prefs::~Prefs(void) {
	MainWindow::getInstance()->removeSystemMenuItem(menuID);
	if (panel)
		delete panel;
	instance = 0;
	panel = 0;
}

void UserPrefs::Prefs::createPrefsPanel(void) {
	if (!panel)
		panel = new Panel(MainWindow::getInstance()->centralWidget());
	panel->show();
}

static Mutex mutex;
UserPrefs::Prefs *UserPrefs::Prefs::instance = 0;

UserPrefs::Prefs *UserPrefs::Prefs::getInstance(void) {
	if (instance)
		return instance;

	/*************************************************************************
	 * Seems like alot of hoops to jump through, but allocation isn't        *
	 *   thread-safe. So effort must be taken to ensure mutual exclusion.    *
	 *************************************************************************/

	Mutex::Locker lock(&::mutex);
	if (!instance)
		instance = new Prefs();

	return instance;
}
