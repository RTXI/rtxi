/*
         The Real-Time eXperiment Interface (RTXI)
         Copyright (C) 2011 Georgia Institute of Technology, University of Utah,
   Weill Cornell Medical College

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
#include <algorithm>
#include <string>

#include "main_window.hpp"

#include <fmt/core.h>

#include "debug.hpp"
#include "rtxiConfig.h"

#include "performance_measurement/performance_measurement.hpp"

MainWindow::MainWindow(Event::Manager* ev_manager)
    : QMainWindow(nullptr, Qt::Window),
      event_manager(ev_manager)
{
  // Make central RTXI parent widget
  mdiArea = new QMdiArea;
  setCentralWidget(mdiArea);

  /* Initialize Window Settings */
  setWindowTitle("RTXI - Real-time eXperimental Interface");
  // setWindowIcon(QIcon("/usr/local/share/rtxi/RTXI-icon.png"));

  /* Set Qt Settings Information */
  QApplication::setOrganizationName("RTXI");
  QApplication::setOrganizationDomain("rtxi.org");
  QApplication::setApplicationName("RTXI");

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

  // create default plugin menu entries
  // this->createSystemMenuItem(
  //     "RT Benchmarks", 
  //     this, 
  //     SLOT(std::make_unique<PerformanceMeasurement::Plugin>()))
}

QAction* MainWindow::insertModuleMenuSeparator()
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

QAction* MainWindow::createModuleMenuItem(const QString& text)
{
  return moduleMenu->addAction(text);
}

QAction* MainWindow::createModuleMenuItem(const QString& text,
                                          const QObject* receiver,
                                          const char* member)
{
  return moduleMenu->addAction(text, receiver, member);
}

void MainWindow::setModuleMenuItemParameter(QAction* action, int parameter)
{
  action->setData(parameter);
}

void MainWindow::clearModuleMenu()
{
  moduleMenu->clear();
}

void MainWindow::changeModuleMenuItem(QAction* action, const QString& text)
{
  action->setText(text);
}

void MainWindow::removeModuleMenuItem(QAction* action)
{
  QList<QAction*> actionList = moduleMenu->actions();
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
  connect(fileMenu,
          SIGNAL(triggered(QAction*)),
          this,
          SLOT(fileMenuActivated(QAction*)));
}

void MainWindow::createModuleMenu()
{
  moduleMenu = menuBar()->addMenu(tr("&Modules"));
  connect(moduleMenu,
          SIGNAL(triggered(QAction*)),
          this,
          SLOT(modulesMenuActivated(QAction*)));
}

void MainWindow::createUtilMenu()
{
  utilMenu = menuBar()->addMenu(tr("&Utilities"));
  filtersSubMenu = utilMenu->addMenu(tr("&Filters"));
  signalsSubMenu = utilMenu->addMenu(tr("&Signals"));
  utilitiesSubMenu = utilMenu->addMenu(tr("&Utilities"));
  connect(utilMenu,
          SIGNAL(triggered(QAction*)),
          this,
          SLOT(utilitiesMenuActivated(QAction*)));

  QDir libsDir("/usr/local/lib/rtxi/");
  if (!libsDir.exists()) {
    return;
  }
  libsDir.setNameFilters(QStringList("*.so"));
  for (const auto& entryItem : libsDir.entryList()) {
    utilItem = new QAction(entryItem, this);
    if (entryItem.contains("analysis") ||
        entryItem.contains("sync") ||
        entryItem.contains("mimic")) {
      utilitiesSubMenu->addAction(utilItem);
    } else if (entryItem.contains("iir") ||
               entryItem.contains("fir")) {
      filtersSubMenu->addAction(utilItem);
    } else if (entryItem.contains("signal")|| 
               entryItem.contains("noise") ||
               entryItem.contains("ttl") ||
               entryItem.contains("maker")) {
      signalsSubMenu->addAction(utilItem);
    } else {
      delete utilItem;
    }
  }
}

void MainWindow::createSystemMenu()
{
  systemMenu = menuBar()->addMenu(tr("&System"));
}

void MainWindow::createWindowsMenu()
{
  windowsMenu = menuBar()->addMenu(tr("&Windows"));
  connect(
      windowsMenu, SIGNAL(aboutToShow()), this, SLOT(windowsMenuAboutToShow()));
}

void MainWindow::createHelpMenu()
{
  helpMenu = menuBar()->addMenu(tr("&Help"));
  helpMenu->addSeparator();
  helpMenu->addAction(artxi);
  helpMenu->addAction(axeno);
  helpMenu->addAction(aqt);
  helpMenu->addSeparator();
  helpMenu->addAction(adocs);
  helpMenu->addAction(sub_issue);
}

void MainWindow::createFileActions()
{
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
  connect(QCoreApplication::instance(),
          SIGNAL(aboutToQuit()),
          mdiArea,
          SLOT(closeAllSubWindows()));
  connect(quit, SIGNAL(triggered()), this, SLOT(close()));
}

void MainWindow::createMdi(QMdiSubWindow* subWindow)
{
  mdiArea->addSubWindow(subWindow);
}

void MainWindow::createHelpActions()
{
  artxi = new QAction(tr("About &RTXI"), this);
  connect(artxi, SIGNAL(triggered()), this, SLOT(about()));

  axeno = new QAction(tr("About &Xenomai"), this);
  connect(axeno, SIGNAL(triggered()), this, SLOT(aboutXeno()));

  aqt = new QAction(tr("About &Qt"), this);
  connect(aqt, SIGNAL(triggered()), this, SLOT(aboutQt()));

  adocs = new QAction(tr("&Documentation"), this);
  connect(adocs, SIGNAL(triggered()), this, SLOT(openDocs()));

  sub_issue = new QAction(tr("&Submit Issue"), this);
  connect(sub_issue, SIGNAL(triggered()), this, SLOT(openSubIssue()));
}

QAction* MainWindow::createSystemMenuItem(const QString& label,
                                          const QObject* handler,
                                          const char* slot)
{
  return systemMenu->addAction(label, handler, slot);
}

void MainWindow::about()
{
  std::string version_str = fmt::format(
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
  QMessageBox::about(this, "About Xenomai", "Running POSIX (non-RT)");
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
  QSettings::setPath(QSettings::NativeFormat,
                     QSettings::SystemScope,
                     "/usr/local/share/rtxi/");
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
  QSettings userprefs;
  QSettings::setPath(QSettings::NativeFormat,
                     QSettings::SystemScope,
                     "/usr/local/share/rtxi/");

  QString filename = QFileDialog::getOpenFileName(
      this,
      tr("Load saved workspace"),
      userprefs.value("/dirs/setfiles", getenv("HOME")).toString(),
      tr("Settings (*.set)"));

  if (QFile(filename).exists()) {
    systemMenu->clear();
    mdiArea->closeAllSubWindows();
    // Settings::Manager::getInstance()->load(filename.toStdString());
  }
}

void MainWindow::saveSettings()
{
  QSettings userprefs;
  QSettings::setPath(QSettings::NativeFormat,
                     QSettings::SystemScope,
                     "/usr/local/share/rtxi/");

  QString filename = QFileDialog::getSaveFileName(
      this,
      tr("Save current workspace"),
      userprefs.value("/dirs/setfiles", getenv("HOME")).toString(),
      tr("Settings (*.set)"));

  if (!filename.isEmpty()) {
    if (!filename.endsWith(".set")) {
      filename = filename + ".set";
    }
    if (QFileInfo(filename).exists()
        && QMessageBox::warning(this,
                                "File Exists",
                                "Do you wish to overwrite " + filename + "?",
                                QMessageBox::Yes,
                                QMessageBox::No)
            != QMessageBox::Yes)
    {
      // DEBUG_MSG ("MainWindow::saveSettings : canceled overwrite\n");
      return;
    }
    // Settings::Manager::getInstance()->save(filename.toStdString());
  }
}

void MainWindow::resetSettings()
{
  // systemMenu->clear();
  // mdiArea->closeAllSubWindows();
  // Settings::Manager::getInstance()->load("/usr/local/share/rtxi/rtxi.conf");
}

void MainWindow::utilitiesMenuActivated(QAction* id)
{
  // Plugin::Manager::getInstance()->load(id->text());
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
    // QAction* item =
    //     new QAction(subWindows.at(i)->widget()->windowTitle(), this);
    windowsMenu->addAction(new QAction(subwin->widget()->windowTitle(), this));
  }
  connect(windowsMenu,
          SIGNAL(triggered(QAction*)),
          this,
          SLOT(windowsMenuActivated(QAction*)));
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

void MainWindow::modulesMenuActivated(QAction* id)
{
  // // Annoying but the best way to do it is to tie an action to the entire
  // menu
  // // so we have to tell it to ignore the first two modules
  // if (id->text().contains("Load Plugin"))
  //   return;

  // // Have to trim the first three characters before loading
  // // or else parser will include qstring formatting
  // Plugin::Manager::getInstance()->load(id->text().remove(0, 3));
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

void MainWindow::closeEvent(QCloseEvent* event)
{
  /*
   * Save MainWindow settings
   */
  QSettings userprefs;
  QSettings::setPath(QSettings::NativeFormat,
                     QSettings::SystemScope,
                     "/usr/local/share/rtxi/");
  userprefs.beginGroup("MainWindow");
  userprefs.setValue("geometry", saveGeometry());
  userprefs.setValue("maximized", isMaximized());
  if (!isMaximized()) {
    userprefs.setValue("pos", pos());
    userprefs.setValue("size", size());
  }
  userprefs.endGroup();
}
