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

#include <QApplication>
#include <QDesktopServices>
#include <QDir>
#include <QFileDialog>
#include <QMenuBar>
#include <QMessageBox>
#include <QSettings>
#include <QString>
#include <QUrl>
#include <string>

#include "main_window.hpp"

#include <fmt/core.h>

#include "connector/connector.hpp"
#include "data_recorder/data_recorder.hpp"
#include "event.hpp"
#include "module_installer/rtxi_wizard.hpp"
#include "oscilloscope/oscilloscope.hpp"
#include "performance_measurement/performance_measurement.hpp"
#include "rtxiConfig.h"
#include "system_control/system_control.hpp"
#include "userprefs/userprefs.hpp"
#include "widgets.hpp"

MainWindow::MainWindow(Event::Manager* ev_manager)
    : QMainWindow(nullptr, Qt::Window)
    , event_manager(ev_manager)
    , mdiArea(new QMdiArea)
{
  // Make central RTXI parent widget
  setCentralWidget(mdiArea);

  /* Initialize Window Settings */
  setWindowTitle("RTXI - Real-time eXperimental Interface");
  setWindowIcon(QIcon("/usr/share/rtxi/RTXI-icon.png"));

  /* Set Qt Settings Information */
  QApplication::setOrganizationName("RTXI");
  QApplication::setOrganizationDomain("rtxi.org");
  QApplication::setApplicationName("RTXI");

  /* Initialize Menus */
  createFileActions();
  createFileMenu();

  /* Initialize Widget Menu */
  createWidgetMenu();

  /* Initialize Utilities menu */
  createUtilMenu();

  /* Initialize System Menu */
  createSystemActions();
  createSystemMenu();

  /* Initialize Windows Menu */
  createWindowsMenu();

  /* Initialize Help Menu */
  createHelpActions();
  createHelpMenu();
}

QAction* MainWindow::insertWidgetMenuSeparator()
{
  return moduleMenu->addSeparator();
}

QAction* MainWindow::createFileMenuItem(const QString& label)
{
  return fileMenu->addAction(label);
}

void MainWindow::clearFileMenu()
{
  // Clear but add back default actions
  fileMenu->clear();
  fileMenu->addAction(load);
  fileMenu->addAction(save);
  fileMenu->addAction(reset);
  fileMenu->addSeparator();
  fileMenu->addAction(quit);
  fileMenu->addSeparator();
}

QAction* MainWindow::createWidgetMenuItem(const QString& text)
{
  return moduleMenu->addAction(text);
}

QAction* MainWindow::createWidgetMenuItem(const QString& text,
                                          const QObject* receiver,
                                          const char* member)
{
  return moduleMenu->addAction(text, receiver, member);
}

void MainWindow::setWidgetMenuItemParameter(QAction* action, int parameter)
{
  action->setData(parameter);
}

void MainWindow::clearWidgetMenu()
{
  moduleMenu->clear();
}

void MainWindow::changeWidgetMenuItem(QAction* action, const QString& text)
{
  action->setText(text);
}

void MainWindow::removeWidgetMenuItem(QAction* action)
{
  const QList<QAction*> actionList = moduleMenu->actions();
  if (!actionList.empty()) {
    moduleMenu->removeAction(action);
  }
}

QAction* MainWindow::createUtilMenuItem(const QString& label,
                                        const QObject* handler,
                                        const char* slot)
{
  return utilMenu->addAction(label, handler, slot);
}

void MainWindow::createFileMenu()
{
  fileMenu = menuBar()->addMenu(tr("&File"));
  fileMenu->addAction(load);
  fileMenu->addAction(save);
  fileMenu->addAction(reset);
  fileMenu->addSeparator();
  fileMenu->addAction(quit);
  fileMenu->addSeparator();
  connect(fileMenu, &QMenu::triggered, this, &MainWindow::fileMenuActivated);
}

void MainWindow::createWidgetMenu()
{
  moduleMenu = menuBar()->addMenu(tr("&Widgets"));
  this->loadDynamicWidget = new QAction("Load Plugin", this);
  moduleMenu->addAction(this->loadDynamicWidget);
  connect(
      moduleMenu, &QMenu::triggered, this, &MainWindow::modulesMenuActivated);
}

void MainWindow::createUtilMenu()
{
  utilMenu = menuBar()->addMenu(tr("&Utilities"));
  filtersSubMenu = utilMenu->addMenu(tr("&Filters"));
  signalsSubMenu = utilMenu->addMenu(tr("&Signals"));
  utilitiesSubMenu = utilMenu->addMenu(tr("&Utilities"));
  connect(
      utilMenu, &QMenu::triggered, this, &MainWindow::utilitiesMenuActivated);

  QDir libsDir = QCoreApplication::applicationDirPath() + QDir::separator()
      + QString("rtxi_modules");
  if (!libsDir.exists()) {
    return;
  }
  libsDir.setNameFilters(QStringList("*.so"));
  for (const auto& entryItem : libsDir.entryList()) {
    utilItem = new QAction(entryItem, this);
    if (entryItem.contains("analysis") || entryItem.contains("sync")
        || entryItem.contains("mimic"))
    {
      utilitiesSubMenu->addAction(utilItem);
    } else if (entryItem.contains("iir") || entryItem.contains("fir")) {
      filtersSubMenu->addAction(utilItem);
    } else if (entryItem.contains("signal") || entryItem.contains("noise")
               || entryItem.contains("ttl") || entryItem.contains("maker"))
    {
      signalsSubMenu->addAction(utilItem);
    } else {
      delete utilItem;
    }
  }
}

void MainWindow::createSystemMenu()
{
  this->systemMenu = menuBar()->addMenu(tr("&System"));
  this->systemMenu->addAction(this->openRTBenchmarks);
  this->systemMenu->addAction(this->openUserPrefs);
  this->systemMenu->addAction(this->openControlPanel);
  this->systemMenu->addAction(this->openConnector);
  this->systemMenu->addAction(this->openOscilloscope);
  this->systemMenu->addAction(this->openDataRecorder);
  this->systemMenu->addAction(this->openRTXIWizard);
  connect(
      systemMenu, &QMenu::triggered, this, &MainWindow::systemMenuActivated);
}

void MainWindow::createWindowsMenu()
{
  windowsMenu = menuBar()->addMenu(tr("&Windows"));
  connect(windowsMenu,
          &QMenu::aboutToShow,
          this,
          &MainWindow::windowsMenuAboutToShow);
}

void MainWindow::createHelpMenu()
{
  this->helpMenu = menuBar()->addMenu(tr("&Help"));
  this->helpMenu->addSeparator();
  this->helpMenu->addAction(artxi);
  this->helpMenu->addAction(axeno);
  this->helpMenu->addAction(aqt);
  this->helpMenu->addSeparator();
  this->helpMenu->addAction(adocs);
  this->helpMenu->addAction(sub_issue);
}

void MainWindow::createFileActions()
{
  this->load = new QAction(tr("&Load Workspace"), this);
  this->load->setShortcuts(QKeySequence::Open);
  this->load->setStatusTip(tr("Load a saved workspace"));
  connect(load, &QAction::triggered, this, &MainWindow::loadSettings);

  this->save = new QAction(tr("&Save Workspace"), this);
  this->save->setShortcuts(QKeySequence::Save);
  this->save->setStatusTip(tr("Save current workspace"));
  connect(save, &QAction::triggered, this, &MainWindow::saveSettings);

  this->reset = new QAction(tr("&Reset Workspace"), this);
  this->reset->setStatusTip(tr("Reset to default RTXI workspace"));
  connect(reset, &QAction::triggered, this, &MainWindow::resetSettings);

  this->quit = new QAction(tr("&Quit"), this);
  this->quit->setShortcut(tr("Ctrl+Q"));
  this->quit->setStatusTip(tr("Quit RTXI"));
  connect(QCoreApplication::instance(),
          &QCoreApplication::aboutToQuit,
          mdiArea,
          &QMdiArea::closeAllSubWindows);
  connect(quit, &QAction::triggered, this, &MainWindow::close);
}

void MainWindow::createHelpActions()
{
  artxi = new QAction(tr("About &RTXI"), this);
  connect(artxi, &QAction::triggered, this, &MainWindow::about);

  axeno = new QAction(tr("About &Xenomai"), this);
  connect(axeno, &QAction::triggered, this, &MainWindow::aboutXeno);

  aqt = new QAction(tr("About &Qt"), this);
  connect(aqt, &QAction::triggered, this, &MainWindow::aboutQt);

  adocs = new QAction(tr("&Documentation"), this);
  connect(adocs, &QAction::triggered, this, &MainWindow::openDocs);

  sub_issue = new QAction(tr("&Submit Issue"), this);
  connect(sub_issue, &QAction::triggered, this, &MainWindow::openSubIssue);
}

void MainWindow::createSystemActions()
{
  this->openRTBenchmarks = new QAction(
      tr(std::string(PerformanceMeasurement::MODULE_NAME).c_str()), this);
  this->openUserPrefs =
      new QAction(tr(std::string(UserPrefs::MODULE_NAME).c_str()), this);
  this->openControlPanel =
      new QAction(tr(std::string(SystemControl::MODULE_NAME).c_str()), this);
  this->openConnector =
      new QAction(tr(std::string(Connector::MODULE_NAME).c_str()), this);
  this->openOscilloscope =
      new QAction(tr(std::string(Oscilloscope::MODULE_NAME).c_str()), this);
  this->openDataRecorder =
      new QAction(tr(std::string(DataRecorder::MODULE_NAME).c_str()), this);
  this->openRTXIWizard =
      new QAction(tr(std::string(RTXIWizard::MODULE_NAME).c_str()), this);
}

void MainWindow::about()
{
  const std::string version_str = fmt::format(
      "{}.{}.{}", RTXI_VERSION_MAJOR, RTXI_VERSION_MINOR, RTXI_VERSION_PATCH);
  QMessageBox::about(
      this,
      "About RTXI",
      QString("RTXI Version ") + QString(version_str.c_str())
          + QString(
              "\n\nReleased under the GPLv3.\nSee www.rtxi.org for details."));
}

void MainWindow::aboutQt()
{
  QMessageBox::aboutQt(this);
}

void MainWindow::aboutXeno()
{
  const std::string realtime = RTXI_RT_CORE == "posix" ? "(non-RT)" : "";
  QMessageBox::about(
      this,
      "About Xenomai",
      fmt::format("Running {}{} core", RTXI_RT_CORE, realtime).c_str());
}

void MainWindow::openDocs()
{
  QDesktopServices::openUrl(QUrl("http://rtxi.org/docs/", QUrl::TolerantMode));
}

void MainWindow::openSubIssue()
{
  QDesktopServices::openUrl(
      QUrl("https://github.com/rtxi/rtxi/issues", QUrl::TolerantMode));
}

/*
 * Load MainWindow settings
 */
void MainWindow::loadWindow()
{
  QSettings userprefs;
  userprefs.beginGroup("MainWindow");
  restoreGeometry(userprefs.value("geometry", saveGeometry()).toByteArray());
  move(userprefs.value("pos", pos()).toPoint());
  resize(userprefs.value("size", size()).toSize());
  if (userprefs.value("maximized", isMaximized()).toBool()) {
    showMaximized();
  }
  userprefs.endGroup();
  show();
}

void MainWindow::loadSettings()
{
  const QSettings userprefs;
  const QString env_var = QString::fromLocal8Bit(qgetenv("HOME"));

  const QString filename = QFileDialog::getOpenFileName(
      this,
      tr("Load saved workspace"),
      userprefs.value("/dirs/setfiles", env_var).toString(),
      tr("Settings (*.set)"));

  if (QFile(filename).exists()) {
    systemMenu->clear();
    mdiArea->closeAllSubWindows();
  }
}

void MainWindow::saveSettings()
{
  QSettings userprefs;
  userprefs.beginGroup("Workspaces");
  auto* save_settings_dialog = new QInputDialog(this);
  save_settings_dialog->setInputMode(QInputDialog::TextInput);
  save_settings_dialog->setComboBoxEditable(true);
  save_settings_dialog->setComboBoxItems(userprefs.childGroups());
  save_settings_dialog->setLabelText("Profile");
  save_settings_dialog->setOkButtonText("Save");
  userprefs.endGroup();
  save_settings_dialog->exec();
}

void MainWindow::resetSettings()
{
  // systemMenu->clear();
  // mdiArea->closeAllSubWindows();
  // Settings::Manager::getInstance()->load("/usr/local/share/rtxi/rtxi.conf");
}

void MainWindow::utilitiesMenuActivated(QAction* id)
{
  this->loadWidget(QCoreApplication::applicationDirPath() + QDir::separator()
                   + QString("rtxi_modules") + QDir::separator() + id->text());
}

void MainWindow::loadWidget(const QString& module_name)
{
  Event::Object event(Event::Type::PLUGIN_INSERT_EVENT);
  event.setParam("pluginName", std::any(module_name.toStdString()));
  this->event_manager->postEvent(&event);

  // If something goes wrong just give up
  auto status = std::any_cast<std::string>(event.getParam("status"));
  if (status == "failure") {
    return;
  }

  auto create_rtxi_panel_func =
      std::any_cast<Widgets::Panel* (*)(QMainWindow*, Event::Manager*)>(
          event.getParam("createRTXIPanel"));
  auto* rtxi_plugin_pointer =
      std::any_cast<Widgets::Plugin*>(event.getParam("pluginPointer"));
  auto* rtxi_panel_pointer = create_rtxi_panel_func(this, this->event_manager);
  rtxi_plugin_pointer->attachPanel(rtxi_panel_pointer);
  // finally plugins can also receive events so make sure to register them
  this->event_manager->registerHandler(rtxi_plugin_pointer);

  // show the panel please
  rtxi_panel_pointer->show();
}

void MainWindow::systemMenuActivated(QAction* id)
{
  this->loadWidget(id->text());
}

void MainWindow::windowsMenuAboutToShow()
{
  // Clear previous entries
  windowsMenu->clear();

  // Add default options
  windowsMenu->addAction(tr("Cascade"), mdiArea, SLOT(cascadeSubWindows()));
  windowsMenu->addAction(tr("Tile"), mdiArea, SLOT(tileSubWindows()));
  windowsMenu->addSeparator();

  // Get list of open subwindows in Mdi Area
  subWindows = mdiArea->subWindowList();

  // Make sure it isn't empty
  if (subWindows.isEmpty()) {
    return;
  }
  // Create windows list based off of what's open
  for (auto* subwin : subWindows) {
    windowsMenu->addAction(new QAction(subwin->widget()->windowTitle(), this));
  }
  connect(
      windowsMenu, &QMenu::triggered, this, &MainWindow::windowsMenuActivated);
}

void MainWindow::windowsMenuActivated(QAction* id)
{
  // Get list of open subwindows in Mdi Area
  subWindows = mdiArea->subWindowList();

  // Make sure it isn't empty
  if (subWindows.isEmpty()) {
    return;
  }
  for (QMdiSubWindow* subwindow : this->subWindows) {
    if (subwindow->widget()->windowTitle() == id->text()) {
      mdiArea->setActiveSubWindow(subwindow);
    }
  }
}

void MainWindow::modulesMenuActivated(QAction* /*unused*/)
{
  const QString filename = QFileDialog::getOpenFileName(
      this,
      tr("Load Plugin"),
      QCoreApplication::applicationDirPath() + QDir::separator()
          + QString("rtxi_modules"),
      tr("Plugins (*.so);;All (*.*)"));
  if (!filename.isNull()) {
    this->loadWidget(filename);
  }
}

void MainWindow::fileMenuActivated(QAction* id)
{
  // Annoying but the best way to do it is to tie an action to the entire menu
  // so we have to tell it to ignore the first three items
  if (id->text().contains("Load Workspace")
      || id->text().contains("Save Workspace")
      || id->text().contains("Reset Workspace") || id->text().contains("Quit"))
  {
    return;
  }

  // Have to trim the first three characters before loading
  // or else parser will include qstring formatting
  systemMenu->clear();
  mdiArea->closeAllSubWindows();
  // Settings::Manager::getInstance()->load(id->text().remove(0,
  // 3).toStdString());
}

void MainWindow::closeEvent(QCloseEvent* /*event*/)
{
  /*
   * Save MainWindow settings
   */
  QSettings userprefs;
  userprefs.beginGroup("MainWindow");
  userprefs.setValue("geometry", saveGeometry());
  userprefs.setValue("maximized", isMaximized());
  if (!isMaximized()) {
    userprefs.setValue("pos", pos());
    userprefs.setValue("size", size());
  }
  userprefs.endGroup();
}
