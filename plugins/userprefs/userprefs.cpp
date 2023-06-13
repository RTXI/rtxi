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
    : Modules::Plugin(ev_manager, mw, std::string(UserPrefs::MODULE_NAME))
{
}

UserPrefs::Panel::Panel(MainWindow* mwindow, Event::Manager* ev_manager)
    : Modules::Panel(std::string(UserPrefs::MODULE_NAME), mwindow, ev_manager)
    , status(new QLabel)
    , subWindow(new QMdiSubWindow)
    , dirGroup(new QGroupBox)
    , HDF(new QGroupBox)
    , buttons(new QGroupBox)
    , settingsDirEdit(new QLineEdit(dirGroup))
    , dataDirEdit(new QLineEdit(dirGroup))
    , HDFBufferEdit(new QLineEdit(HDF))
{
  // Make Mdi

  subWindow->setWindowIcon(QIcon("/usr/local/share/rtxi/RTXI-widget-icon.png"));
  subWindow->setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowCloseButtonHint
                            | Qt::WindowMinimizeButtonHint);
  subWindow->setAttribute(Qt::WA_DeleteOnClose);
  mwindow->createMdi(subWindow);

  // Preferences structure
  const QSettings user_preferences;
  QSettings::setPath(QSettings::NativeFormat,
                     QSettings::SystemScope,
                     "/usr/local/share/rtxi/");

  // Main layout
  auto* box_layout = new QVBoxLayout;

  // Create child widget and layout
  auto* dirLayout = new QGridLayout;

  // Create elements for directory paths
  const QString env_var = QString::fromLocal8Bit(qgetenv("HOME"));
  settingsDirEdit->setText(
      user_preferences.value("/dirs/setfiles", env_var).toString());
  dirLayout->addWidget(
      new QLabel(tr("Default settings directory:")), 0, 0, 1, 1);
  dirLayout->addWidget(settingsDirEdit, 0, 1, 1, 1);
  auto* chooseSettingsDirButton = new QPushButton("Browse", this);
  dirLayout->addWidget(chooseSettingsDirButton, 0, 2, 1, 2);
  QObject::connect(chooseSettingsDirButton,
                   SIGNAL(released(void)),
                   this,
                   SLOT(chooseSettingsDir(void)));

  dataDirEdit->setText(
      user_preferences.value("/dirs/data", env_var).toString());
  dirLayout->addWidget(
      new QLabel(tr("Default HDF5 data directory:")), 1, 0, 1, 1);
  dirLayout->addWidget(dataDirEdit, 1, 1, 1, 1);
  auto* chooseDataDirButton = new QPushButton("Browse", this);
  dirLayout->addWidget(chooseDataDirButton, 1, 2, 1, 2);
  QObject::connect(chooseDataDirButton,
                   SIGNAL(released(void)),
                   this,
                   SLOT(chooseDataDir(void)));

  // Attach layout to group
  dirGroup->setLayout(dirLayout);

  // Create new child widget and layout
  auto* hdfLayout = new QGridLayout;

  // Create elements for child widget
  hdfLayout->addWidget(
      new QLabel(tr("HDF Data Recorder Buffer Size (MB):")), 0, 0, 1, 1);
  hdfLayout->addWidget(HDFBufferEdit, 0, 1, 1, 1);
  HDFBufferEdit->setText(
      QString::number(user_preferences.value("/system/HDFbuffer", 10).toInt()));

  // Attach child to parent
  HDF->setLayout(hdfLayout);

  // Create new child widget
  auto* buttonLayout = new QHBoxLayout;

  // Create elements for child widget
  auto* resetButton = new QPushButton("Reset");
  QObject::connect(
      resetButton, SIGNAL(released(void)), this, SLOT(reset(void)));
  auto* applyButton = new QPushButton("Save");
  QObject::connect(
      applyButton, SIGNAL(released(void)), this, SLOT(apply(void)));
  auto* cancelButton = new QPushButton("Close");
  QObject::connect(
      cancelButton, SIGNAL(released(void)), subWindow, SLOT(close(void)));

  status->setText("Defaults \nloaded");
  // NOLINTNEXTLINE
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
  box_layout->addWidget(dirGroup);
  box_layout->addWidget(HDF);
  box_layout->addWidget(buttons);

  // Attach layout to widget
  setLayout(box_layout);
  setWindowTitle(QString::fromStdString(this->getName()));

  // Set layout to Mdi
  subWindow->setWidget(this);
  subWindow->setFixedSize(500, subWindow->sizeHint().height());
  show();
}

void UserPrefs::Panel::reset()
{
  const QString env_var = QString::fromLocal8Bit(qgetenv("HOME"));
  settingsDirEdit->setText(env_var);
  dataDirEdit->setText(env_var);
  HDFBufferEdit->setText(QString::number(10));
  userprefs.setValue("/dirs/setfiles", settingsDirEdit->text());
  userprefs.setValue("/dirs/data", dataDirEdit->text());
  bool ok = false;
  const QString buffer = HDFBufferEdit->text();
  userprefs.setValue("/system/HDFbuffer", buffer.toInt(&ok));
  status->setText("Preferences \nreset");
}

void UserPrefs::Panel::apply()
{
  userprefs.setValue("/dirs/setfiles", settingsDirEdit->text());
  userprefs.setValue("/dirs/data", dataDirEdit->text());
  bool ok = false;
  const QString buffer = HDFBufferEdit->text();
  userprefs.setValue("/system/HDFbuffer", buffer.toInt(&ok));
  status->setText("Preferences \napplied");
}

void UserPrefs::Panel::chooseSettingsDir()
{
  const QString env_var = QString::fromLocal8Bit(qgetenv("HOME"));
  const QString dir_name = QFileDialog::getExistingDirectory(
      this,
      tr("Choose default directory for settings files"),
      userprefs.value("/dirs/setfiles", env_var).toString(),
      QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
  settingsDirEdit->setText(dir_name);
}

void UserPrefs::Panel::chooseDataDir()
{
  const QString env_var = QString::fromLocal8Bit(qgetenv("HOME"));
  const QString dir_name = QFileDialog::getExistingDirectory(
      this,
      tr("Choose default directory for HDF5 data files"),
      userprefs.value("/dirs/data", env_var).toString(),
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
    Modules::Plugin* /*host_plugin*/)
{
  return {nullptr};
}

Modules::FactoryMethods UserPrefs::getFactories()
{
  Modules::FactoryMethods fact;
  fact.createPanel = &UserPrefs::createRTXIPanel;
  fact.createComponent = &UserPrefs::createRTXIComponent;
  fact.createPlugin = &UserPrefs::createRTXIPlugin;
  return fact;
}
