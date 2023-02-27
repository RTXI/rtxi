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

/*
 * The settings are only written when the panel is closed and the QSettings
 * is destructed. So all Reset, Apply, and Cancel buttons all close the panel.
 */

#include "userprefs.h"

#include "debug.hpp"
#include "main_window.hpp"

UserPrefs::Plugin::Plugin(Event::Manager* ev_manager, MainWindow* mw)
    : Modules::Plugin(ev_manager, mw, "User Preferences")
{
}

UserPrefs::Panel::Panel(MainWindow* main_window, Event::Manager* ev_manager)
    : Modules::Panel("User Preferences", main_window, ev_manager)
{
  // Make Mdi
  subWindow = new QMdiSubWindow;
  subWindow->setWindowIcon(QIcon("/usr/local/share/rtxi/RTXI-widget-icon.png"));
  subWindow->setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowCloseButtonHint
                            | Qt::WindowMinimizeButtonHint);
  subWindow->setAttribute(Qt::WA_DeleteOnClose);
  main_window->createMdi(subWindow);

  // Preferences structure
  QSettings userprefs;
  userprefs.setPath(QSettings::NativeFormat,
                    QSettings::SystemScope,
                    "/usr/local/share/rtxi/");

  // Main layout
  QVBoxLayout* layout = new QVBoxLayout;

  // Create child widget and layout
  dirGroup = new QGroupBox;
  QGridLayout* dirLayout = new QGridLayout;

  // Create elements for directory paths
  settingsDirEdit = new QLineEdit(dirGroup);
  settingsDirEdit->setText(
      userprefs.value("/dirs/setfiles", getenv("HOME")).toString());
  dirLayout->addWidget(
      new QLabel(tr("Default settings directory:")), 0, 0, 1, 1);
  dirLayout->addWidget(settingsDirEdit, 0, 1, 1, 1);
  QPushButton* chooseSettingsDirButton = new QPushButton("Browse", this);
  dirLayout->addWidget(chooseSettingsDirButton, 0, 2, 1, 2);
  QObject::connect(chooseSettingsDirButton,
                   SIGNAL(released(void)),
                   this,
                   SLOT(chooseSettingsDir(void)));

  dataDirEdit = new QLineEdit(dirGroup);
  dataDirEdit->setText(
      userprefs.value("/dirs/data", getenv("HOME")).toString());
  dirLayout->addWidget(
      new QLabel(tr("Default HDF5 data directory:")), 1, 0, 1, 1);
  dirLayout->addWidget(dataDirEdit, 1, 1, 1, 1);
  QPushButton* chooseDataDirButton = new QPushButton("Browse", this);
  dirLayout->addWidget(chooseDataDirButton, 1, 2, 1, 2);
  QObject::connect(chooseDataDirButton,
                   SIGNAL(released(void)),
                   this,
                   SLOT(chooseDataDir(void)));

  // Attach layout to group
  dirGroup->setLayout(dirLayout);

  // Create new child widget and layout
  HDF = new QGroupBox;
  QGridLayout* hdfLayout = new QGridLayout;

  // Create elements for child widget
  HDFBufferEdit = new QLineEdit(HDF);
  hdfLayout->addWidget(
      new QLabel(tr("HDF Data Recorder Buffer Size (MB):")), 0, 0, 1, 1);
  hdfLayout->addWidget(HDFBufferEdit, 0, 1, 1, 1);
  HDFBufferEdit->setText(
      QString::number(userprefs.value("/system/HDFbuffer", 10).toInt()));

  // Attach child to parent
  HDF->setLayout(hdfLayout);

  // Create new child widget
  buttons = new QGroupBox;
  QHBoxLayout* buttonLayout = new QHBoxLayout;

  // Create elements for child widget
  QPushButton* resetButton = new QPushButton("Reset");
  QObject::connect(
      resetButton, SIGNAL(released(void)), this, SLOT(reset(void)));
  QPushButton* applyButton = new QPushButton("Save");
  QObject::connect(
      applyButton, SIGNAL(released(void)), this, SLOT(apply(void)));
  QPushButton* cancelButton = new QPushButton("Close");
  QObject::connect(
      cancelButton, SIGNAL(released(void)), subWindow, SLOT(close(void)));
  status = new QLabel;
  status->setText("Defaults \nloaded");
  status->setFrameStyle(QFrame::Panel | QFrame::Sunken);
  status->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

  // Assign elements to layout in child widget
  buttonLayout->addWidget(resetButton);
  buttonLayout->addWidget(applyButton);
  buttonLayout->addWidget(cancelButton);
  buttonLayout->addWidget(status);

  // Assign layout to child widget
  buttons->setLayout(buttonLayout);

  // Attach child widget to parent widget
  layout->addWidget(dirGroup);
  layout->addWidget(HDF);
  layout->addWidget(buttons);

  // Attach layout to widget
  setLayout(layout);
  setWindowTitle(QString::fromStdString(this->getName()));

  // Set layout to Mdi
  subWindow->setWidget(this);
  subWindow->setFixedSize(500, subWindow->sizeHint().height());
  show();
}

void UserPrefs::Panel::reset()
{
  settingsDirEdit->setText(getenv("HOME"));
  dataDirEdit->setText(getenv("HOME"));
  HDFBufferEdit->setText(QString::number(10));
  userprefs.setValue("/dirs/setfiles", settingsDirEdit->text());
  userprefs.setValue("/dirs/data", dataDirEdit->text());
  bool ok;
  QString buffer = HDFBufferEdit->text();
  userprefs.setValue("/system/HDFbuffer", buffer.toInt(&ok));
  status->setText("Preferences \nreset");
}

void UserPrefs::Panel::apply()
{
  userprefs.setValue("/dirs/setfiles", settingsDirEdit->text());
  userprefs.setValue("/dirs/data", dataDirEdit->text());
  bool ok;
  QString buffer = HDFBufferEdit->text();
  userprefs.setValue("/system/HDFbuffer", buffer.toInt(&ok));
  status->setText("Preferences \napplied");
}

void UserPrefs::Panel::chooseSettingsDir()
{
  QString dir_name = QFileDialog::getExistingDirectory(
      this,
      tr("Choose default directory for settings files"),
      userprefs.value("/dirs/setfiles", getenv("HOME")).toString(),
      QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
  settingsDirEdit->setText(dir_name);
}

void UserPrefs::Panel::chooseDataDir()
{
  QString dir_name = QFileDialog::getExistingDirectory(
      this,
      tr("Choose default directory for HDF5 data files"),
      userprefs.value("/dirs/data", getenv("HOME")).toString(),
      QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
  dataDirEdit->setText(dir_name);
}

std::unique_ptr<Modules::Plugin> UserPrefs::createRTXIPlugin(
    Event::Manager* ev_manager, MainWindow* main_window)
{
  return std::make_unique<UserPrefs::Plugin>(ev_manager, main_window);
}

Modules::Panel* UserPrefs::createRTXIPanel(MainWindow* main_window,
                                           Event::Manager* ev_manager)
{
  return static_cast<Modules::Panel*>(
      new UserPrefs::Panel(main_window, ev_manager));
}

std::unique_ptr<Modules::Component> UserPrefs::createRTXIComponent(
    Modules::Plugin* host_plugin)
{
  return std::unique_ptr<Modules::Component>(nullptr);
}

Modules::FactoryMethods UserPrefs::getFactories()
{
  Modules::FactoryMethods fact;
  fact.createPanel = &UserPrefs::createRTXIPanel;
  fact.createComponent = &UserPrefs::createRTXIComponent;
  fact.createPlugin = &UserPrefs::createRTXIPlugin;
  return fact;
}
