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

/*
 * The settings are only written when the panel is closed and the QSettings
 * is destructed. So all Reset, Apply, and Cancel buttons all close the panel.
 */

#include <debug.h>
#include <main_window.h>
#include <userprefs.h>
#include <QtGui>
#include <QFileDialog>

UserPrefs::Panel::Panel(QWidget *parent) : QWidget(parent) {
	setAttribute(Qt::WA_DeleteOnClose);
  QBoxLayout *layout = new QVBoxLayout(this);

  setWindowTitle("RTXI User Preferences");

  // load QSettings
  QSettings userprefs;
  userprefs.setPath(QSettings::NativeFormat, QSettings::UserScope, "RTXI");

  // create GUI elements and set text
  QGroupBox *dirGroup;
  dirGroup = new QGroupBox();

  QWidget *hbox;
  hbox = new QWidget(dirGroup);
  (void) (new QLabel("Default settings directory:", hbox))->setFixedWidth(250);
  settingsDirEdit = new QLineEdit(hbox);
  settingsDirEdit->setFixedWidth(200);
  settingsDirEdit->setText(userprefs.value("/dirs/setfiles", getenv("HOME")).toString());
  QPushButton *chooseSettingsDirButton = new QPushButton("Browse", hbox);
  QObject::connect(chooseSettingsDirButton,SIGNAL(clicked(void)),this,SLOT(chooseSettingsDir(void)));

  hbox = new QWidget(dirGroup);
  (void) (new QLabel("Default DYNAMO models directory:", hbox))->setFixedWidth(
      250);
  dynamoDirEdit = new QLineEdit(hbox);
  dynamoDirEdit->setText(userprefs.value("/dirs/dynamomodels", getenv("HOME")).toString());
  QPushButton *chooseDynamoDirButton = new QPushButton("Browse", hbox);
  QObject::connect(chooseDynamoDirButton,SIGNAL(clicked(void)),this,SLOT(chooseDynamoDir(void)));

  hbox = new QWidget(dirGroup);
  (void) (new QLabel("Default HDF5 data directory:", hbox))->setFixedWidth(250);
  dataDirEdit = new QLineEdit(hbox);
  dataDirEdit->setText(userprefs.value("/dirs/data", getenv("HOME")).toString());
  QPushButton *chooseDataDirButton = new QPushButton("Browse", hbox);
  QObject::connect(chooseDataDirButton,SIGNAL(clicked(void)),this,SLOT(chooseDataDir(void)));

  layout->addWidget(dirGroup);

  hbox = new QWidget(this);
  (void) (new QLabel("HDF Data Recorder Buffer Size (MB):", hbox))->setFixedWidth(250);
  HDFBufferEdit = new QLineEdit(hbox);
  HDFBufferEdit->setText(QString::number(userprefs.value("/system/HDFbuffer", 10).toInt()));

  layout->addWidget(hbox);

  hbox = new QWidget(this);

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
  HDFBufferEdit->setText(QString::number(10));
  apply();
}

void UserPrefs::Panel::apply(void) {
  userprefs.setValue("/dirs/setfiles", settingsDirEdit->text());
  userprefs.setValue("/dirs/dynamomodels", dynamoDirEdit->text());
  userprefs.setValue("/dirs/data", dataDirEdit->text());
  bool ok;
  QString buffer = HDFBufferEdit->text();
  userprefs.setValue("/system/HDFbuffer", buffer.toInt(&ok));
  this->close();
}

void UserPrefs::Panel::cancel(void) {
  this->close();
}

void UserPrefs::Panel::chooseSettingsDir(void) {
  QString dir_name = QFileDialog::getExistingDirectory(this, tr("Choose default directory for settings files"), 
  		userprefs.value("/dirs/setfiles", getenv("HOME")).toString(),
  		QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
  settingsDirEdit->setText(dir_name);
}

void UserPrefs::Panel::chooseDynamoDir(void) {
  QString dir_name = QFileDialog::getExistingDirectory(this, tr("Choose default directory for DYNAMO models"), 
  		userprefs.value("/dirs/setfiles", getenv("HOME")).toString(),
  		QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
  dynamoDirEdit->setText(dir_name);
}

void UserPrefs::Panel::chooseDataDir(void) {
  QString dir_name = QFileDialog::getExistingDirectory(this, tr("Choose default directory for HDF5 data files"), 
  		userprefs.value("/dirs/setfiles", getenv("HOME")).toString(),
  		QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
  dataDirEdit->setText(dir_name);
}

extern "C" Plugin::Object * createRTXIPlugin(void *) {
  return UserPrefs::Prefs::getInstance();
}

UserPrefs::Prefs::Prefs(void) : panel(0) {
	MainWindow::getInstance()->createSystemMenuItem("Preferences",this,SLOT(createPrefsPanel(void)));
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

UserPrefs::Prefs *
UserPrefs::Prefs::getInstance(void) {
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
