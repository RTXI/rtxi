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
#include <QDataStream>
#include <QDesktopServices>
#include <QDir>
#include <QFileDialog>
#include <QInputDialog>
#include <QMdiArea>
#include <QMdiSubWindow>
#include <QMenuBar>
#include <QMessageBox>
#include <QSettings>
#include <QString>
#include <QUrl>
#include <cstddef>
#include <string>
#include <unordered_map>

#include "main_window.hpp"

#include <fmt/core.h>

#include "connector/connector.hpp"
#include "daq.hpp"
#include "data_recorder/data_recorder.hpp"
#include "event.hpp"
#include "module_installer/rtxi_wizard.hpp"
#include "oscilloscope/oscilloscope.hpp"
#include "performance_measurement/performance_measurement.hpp"
#include "rt.hpp"
#include "rtos.hpp"
#include "rtxiConfig.h"
#include "system_control/system_control.hpp"
#include "userprefs/userprefs.hpp"
#include "widgets.hpp"

// This is defined here because top level function is the only other class
// in entire RTXI that needs to deal with connections, but I don't want to
// use block pointers directly here
// NOTE: In order to save this for serialization, we need to define stream
// operators so that Qt can serialize it for use inside the settings, and
// deserialize it back when loading settings.
struct plugin_connection
{
  qint32 src_id;
  qint32 src_direction;
  qint32 src_port;
  qint32 dest_id;
  qint32 dest_port;
};

QDataStream& operator<<(QDataStream& out, const plugin_connection& conn)
{
  out << conn.src_id;
  out << conn.src_direction;
  out << conn.src_port;
  out << conn.dest_id;
  out << conn.dest_port;
  return out;
}

QDataStream& operator>>(QDataStream& in, plugin_connection& conn)
{
  in >> conn.dest_port;
  in >> conn.dest_id;
  in >> conn.src_port;
  in >> conn.src_direction;
  in >> conn.src_id;
  return in;
}

Q_DECLARE_METATYPE(plugin_connection)

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

  // Define a custom type to qt type system for settings management
  qRegisterMetaType<plugin_connection>();
  qRegisterMetaTypeStreamOperators<plugin_connection>();
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
  userprefs.beginGroup("settings");
  const auto workspace_dir_key = QString::fromStdString(
      std::string(UserPrefs::WORKSPACE_SAVE_LOCATION_KEY));
  const auto data_dir_key =
      QString::fromStdString(std::string(UserPrefs::HDF5_SAVE_LOCATION_KEY));
  auto workspace_dir = userprefs.value(workspace_dir_key).toString();
  auto data_dir = userprefs.value(data_dir_key).toString();
  const QString env_var = QString::fromLocal8Bit(qgetenv("HOME"));
  if (workspace_dir == "") {
    userprefs.setValue(workspace_dir_key, env_var);
  }
  if (data_dir == "") {
    userprefs.setValue(data_dir_key, env_var);
  }
  userprefs.endGroup();

  show();
}

void MainWindow::savePeriodSettings(QSettings& userprefs)
{
  Event::Object get_period_event(Event::Type::RT_GET_PERIOD_EVENT);
  event_manager->postEvent(&get_period_event);
  const auto period =
      std::any_cast<int64_t>(get_period_event.getParam("period"));
  userprefs.setValue("period", QString::number(period));
}

void MainWindow::loadPeriodSettings(QSettings& userprefs)
{
  const auto period =
      userprefs.value("period", QVariant::fromValue(RT::OS::DEFAULT_PERIOD))
          .value<int64_t>();
  Event::Object event(Event::Type::RT_PERIOD_EVENT);
  event.setParam("period", std::any(period));
  this->event_manager->postEvent(&event);
}

void MainWindow::saveDAQSettings(QSettings& userprefs)
{
  userprefs.beginGroup("DAQs");

  Event::Object get_devices_event(Event::Type::DAQ_DEVICE_QUERY_EVENT);
  this->event_manager->postEvent(&get_devices_event);
  auto devices = std::any_cast<std::vector<DAQ::Device*>>(
      get_devices_event.getParam("devices"));
  QString dev_name;
  for (const auto& device : devices) {
    dev_name = QString::fromStdString(device->getName());
    userprefs.beginGroup(QString::number(device->getID()));
    userprefs.setValue("name", QString::fromStdString(device->getName()));
    userprefs.beginGroup("AI");
    for (size_t ai_channel = 0;
         ai_channel < device->getChannelCount(DAQ::ChannelType::AI);
         ++ai_channel)
    {
      userprefs.beginGroup(QString::number(ai_channel));
      userprefs.setValue(
          "active", device->getChannelActive(DAQ::ChannelType::AI, ai_channel));
      userprefs.setValue("downsample",
                         static_cast<quint64>(device->getAnalogDownsample(
                             DAQ::ChannelType::AI, ai_channel)));
      userprefs.setValue(
          "gain", device->getAnalogGain(DAQ::ChannelType::AI, ai_channel));
      userprefs.setValue(
          "offset",
          device->getAnalogZeroOffset(DAQ::ChannelType::AI, ai_channel));
      userprefs.setValue("range",
                         static_cast<quint64>(device->getAnalogRange(
                             DAQ::ChannelType::AI, ai_channel)));
      userprefs.setValue("reference",
                         static_cast<quint64>(device->getAnalogReference(
                             DAQ::ChannelType::AI, ai_channel)));
      userprefs.setValue("units",
                         static_cast<quint64>(device->getAnalogUnits(
                             DAQ::ChannelType::AI, ai_channel)));
      userprefs.endGroup();  // channel
    }
    userprefs.endGroup();  // Analog Input
    userprefs.beginGroup("AO");
    for (size_t ao_channel = 0;
         ao_channel < device->getChannelCount(DAQ::ChannelType::AO);
         ++ao_channel)
    {
      userprefs.beginGroup(QString::number(ao_channel));
      userprefs.setValue(
          "active", device->getChannelActive(DAQ::ChannelType::AO, ao_channel));
      userprefs.setValue("downsample",
                         static_cast<quint64>(device->getAnalogDownsample(
                             DAQ::ChannelType::AO, ao_channel)));
      userprefs.setValue(
          "gain", device->getAnalogGain(DAQ::ChannelType::AO, ao_channel));
      userprefs.setValue(
          "offset",
          device->getAnalogZeroOffset(DAQ::ChannelType::AO, ao_channel));
      userprefs.setValue("range",
                         static_cast<quint64>(device->getAnalogRange(
                             DAQ::ChannelType::AO, ao_channel)));
      userprefs.setValue("reference",
                         static_cast<quint64>(device->getAnalogReference(
                             DAQ::ChannelType::AO, ao_channel)));
      userprefs.setValue("units",
                         static_cast<quint64>(device->getAnalogUnits(
                             DAQ::ChannelType::AO, ao_channel)));
      userprefs.endGroup();
    }
    userprefs.endGroup();  // Analog Output
    userprefs.beginGroup("DI");
    for (size_t di_channel = 0;
         di_channel < device->getChannelCount(DAQ::ChannelType::DI);
         ++di_channel)
    {
      userprefs.beginGroup(QString::number(di_channel));
      userprefs.setValue(
          "active", device->getChannelActive(DAQ::ChannelType::DI, di_channel));
      userprefs.endGroup();
    }
    userprefs.endGroup();  // Digital Input
    userprefs.beginGroup("DO");
    for (size_t do_channel = 0;
         do_channel < device->getChannelCount(DAQ::ChannelType::DO);
         ++do_channel)
    {
      userprefs.beginGroup(QString::number(do_channel));
      userprefs.setValue(
          "active", device->getChannelActive(DAQ::ChannelType::DO, do_channel));
      userprefs.endGroup();
    }
    userprefs.endGroup();  // Digital Output
    userprefs.endGroup();  // Device ID
  }
  userprefs.endGroup();  // DAQ
}

void MainWindow::loadDAQSettings(
    QSettings& userprefs, std::unordered_map<int, IO::Block*>& block_cache)
{
  userprefs.beginGroup("DAQs");
  Event::Object get_devices_event(Event::Type::DAQ_DEVICE_QUERY_EVENT);
  this->event_manager->postEvent(&get_devices_event);
  auto devices = std::any_cast<std::vector<DAQ::Device*>>(
      get_devices_event.getParam("devices"));
  QString device_name;
  QString channel_name;
  DAQ::Device* tmp_device = nullptr;
  DAQ::index_t current_channel_id = 0;
  for (const auto& device_id : userprefs.childGroups()) {
    userprefs.beginGroup(device_id);
    device_name = userprefs.value("name").toString();
    // NOTE: We need to make sure that the settings we are about to load are
    // not reloaded for other devices. Therefore we check whether the name
    // exists in the loaded devices registry and whether we already used
    // the block in a previous iteration (could be the case when using multiple
    // devices of the same type)
    auto iter =
        std::find_if(devices.begin(),
                     devices.end(),
                     [device_name, device_id, block_cache](IO::Block* block)
                     {
                       return (block->getName() == device_name.toStdString())
                           && (std::find_if(block_cache.begin(),
                                            block_cache.end(),
                                            [block](const auto& entry)
                                            { return entry.second == block; })
                               == block_cache.end());
                     });
    if (iter == devices.end()) {
      ERROR_MSG("Unable to find DAQ device {} from the list of loaded devices.",
                device_name.toStdString());
      userprefs.endGroup();  // device
      continue;
    }
    tmp_device = *iter;
    userprefs.beginGroup("AI");
    for (const auto& channel_id : userprefs.childGroups()) {
      current_channel_id = channel_id.toUInt();
      userprefs.beginGroup(channel_id);
      tmp_device->setChannelActive(DAQ::ChannelType::AI,
                                   current_channel_id,
                                   userprefs.value("active").value<bool>());
      tmp_device->setAnalogDownsample(
          DAQ::ChannelType::AI,
          current_channel_id,
          userprefs.value("downsample").value<quint64>());
      tmp_device->setAnalogGain(DAQ::ChannelType::AI,
                                current_channel_id,
                                userprefs.value("gain").value<double>());
      tmp_device->setAnalogZeroOffset(
          DAQ::ChannelType::AI,
          current_channel_id,
          userprefs.value("offset").value<double>());
      tmp_device->setAnalogRange(DAQ::ChannelType::AI,
                                 current_channel_id,
                                 userprefs.value("range").value<quint64>());
      tmp_device->setAnalogReference(
          DAQ::ChannelType::AI,
          current_channel_id,
          userprefs.value("reference").value<quint64>());
      tmp_device->setAnalogUnits(DAQ::ChannelType::AI,
                                 current_channel_id,
                                 userprefs.value("units").value<quint64>());
      userprefs.endGroup();  // channel
    }
    userprefs.endGroup();  // Analog Input
    userprefs.beginGroup("AO");

    for (const auto& channel_id : userprefs.childGroups()) {
      current_channel_id = channel_id.toUInt();
      userprefs.beginGroup(channel_id);
      tmp_device->setChannelActive(DAQ::ChannelType::AO,
                                   current_channel_id,
                                   userprefs.value("active").value<bool>());
      tmp_device->setAnalogDownsample(
          DAQ::ChannelType::AO,
          current_channel_id,
          userprefs.value("downsample").value<quint64>());
      tmp_device->setAnalogGain(DAQ::ChannelType::AO,
                                current_channel_id,
                                userprefs.value("gain").value<double>());
      tmp_device->setAnalogZeroOffset(
          DAQ::ChannelType::AO,
          current_channel_id,
          userprefs.value("offset").value<double>());
      tmp_device->setAnalogRange(DAQ::ChannelType::AO,
                                 current_channel_id,
                                 userprefs.value("range").value<quint64>());
      tmp_device->setAnalogReference(
          DAQ::ChannelType::AO,
          current_channel_id,
          userprefs.value("reference").value<quint64>());
      tmp_device->setAnalogUnits(DAQ::ChannelType::AO,
                                 current_channel_id,
                                 userprefs.value("units").value<quint64>());
      userprefs.endGroup();  // channel
    }
    userprefs.endGroup();  // Analog Output
    userprefs.beginGroup("DI");
    for (const auto& channel_id : userprefs.childGroups()) {
      current_channel_id = channel_id.toUInt();
      userprefs.beginGroup(channel_id);
      tmp_device->setActive(userprefs.value("active").value<bool>());
      userprefs.endGroup();
    }
    userprefs.endGroup();  // Digital Input
    userprefs.beginGroup("DO");
    for (const auto& channel_id : userprefs.childGroups()) {
      current_channel_id = channel_id.toUInt();
      userprefs.beginGroup(channel_name);
      tmp_device->setActive(userprefs.value("active").value<bool>());
      userprefs.endGroup();
    }
    userprefs.endGroup();  // Digital Output
    userprefs.endGroup();  // Device name
    block_cache[device_id.toInt()] = tmp_device;
  }
  userprefs.endGroup();  // DAQ
}

void MainWindow::saveWidgetSettings(QSettings& userprefs)
{
  userprefs.beginGroup("Widgets");
  Event::Object loaded_plugins_query(Event::Type::PLUGIN_LIST_QUERY_EVENT);
  this->event_manager->postEvent(&loaded_plugins_query);
  const auto plugin_list = std::any_cast<std::vector<const Widgets::Plugin*>>(
      loaded_plugins_query.getParam("plugins"));
  int non_component_plugin_id = -1;
  int plugin_id = 0;
  for (const auto& entry : plugin_list) {
    // We don't expect the number of blocks in the system to exceed the max
    // positive value of int In 2025 you would run out of memory first before
    // that happens. Additionally we are saving negative values to indicate to
    // the workspace loading system that the negative valued plugins do not
    // contain a component and therefore should be left out of the connection
    // loading step.
    plugin_id = entry->hasComponent() ? static_cast<int>(entry->getID())
                                      : non_component_plugin_id--;
    userprefs.beginGroup(QString::number(plugin_id));
    userprefs.setValue("library", QString::fromStdString(entry->getLibrary()));
    userprefs.beginGroup("standardParams");
    entry->saveParameterSettings(userprefs);
    userprefs.endGroup();  // standardParams
    userprefs.beginGroup("customParams");
    entry->saveCustomParameterSettings(userprefs);
    userprefs.endGroup();  // customParams
    userprefs.endGroup();  // widget count
  }
  userprefs.endGroup();  // Widgets
}

void MainWindow::loadWidgetSettings(
    QSettings& userprefs, std::unordered_map<int, IO::Block*>& block_cache)
{
  userprefs.beginGroup("Widgets");
  QString plugin_name;
  Widgets::Plugin* plugin_ptr = nullptr;
  std::string event_status;
  for (const auto& plugin_instance_id : userprefs.childGroups()) {
    userprefs.beginGroup(plugin_instance_id);
    plugin_name = userprefs.value("library").toString();
    this->loadWidget(plugin_name, plugin_ptr);
    // Load the settings
    userprefs.beginGroup("standardParams");
    plugin_ptr->loadParameterSettings(userprefs);
    userprefs.endGroup();  // standardParams
    userprefs.beginGroup("customParams");
    plugin_ptr->loadCustomParameterSettings(userprefs);
    userprefs.endGroup();  // customParams
    userprefs.endGroup();  // plugin_instance_id
    block_cache[plugin_instance_id.toInt()] = plugin_ptr->getBlock();
  }
  userprefs.endGroup();  // Widgets
}

void MainWindow::saveConnectionSettings(QSettings& userprefs)
{
  Event::Object all_connections_event(
      Event::Type::IO_ALL_CONNECTIONS_QUERY_EVENT);
  event_manager->postEvent(&all_connections_event);
  auto connections = std::any_cast<std::vector<RT::block_connection_t>>(
      all_connections_event.getParam("connections"));
  plugin_connection id_connection {};
  userprefs.beginGroup("Connections");
  int connection_count = 0;
  for (const auto& conn : connections) {
    // NOTE: We don't handle connections that are managed by plugins
    // themselves. Namely connections related to probes from oscilloscope and
    // recorders from the recorder plugin
    if (conn.dest->getName().find("Probe") != std::string::npos
        || conn.dest->getName().find("Recording") != std::string::npos)
    {
      continue;
    }
    // We need to match the block id with the id already stored in the
    // settings file when saveWidgets was called.
    id_connection.src_id = static_cast<int>(conn.src->getID());
    id_connection.src_direction = static_cast<int>(conn.src_port_type);
    id_connection.src_port = static_cast<int>(conn.src_port);
    id_connection.dest_id = static_cast<int>(conn.dest->getID());
    id_connection.dest_port = static_cast<int>(conn.dest_port);
    userprefs.setValue(QString::number(connection_count++),
                       QVariant::fromValue(id_connection));
  }
  userprefs.endGroup();  // Connections
}

void MainWindow::loadConnectionSettings(
    QSettings& userprefs, std::unordered_map<int, IO::Block*>& block_cache)
{
  ///////////////////// Load connections /////////////////////////
  RT::block_connection_t connection;
  std::vector<Event::Object> connection_events;
  plugin_connection id_connection {};
  userprefs.beginGroup("Connections");
  for (const auto& conn_count : userprefs.childKeys()) {
    id_connection = userprefs.value(conn_count).value<plugin_connection>();
    if (block_cache.find(id_connection.src_id) == block_cache.end()
        || block_cache.find(id_connection.dest_id) == block_cache.end())
    {
      ERROR_MSG(
          "MainWindow::loadWidgetSettings : invalid connection found. "
          "Skipping");
      continue;
    }
    connection.src = block_cache[id_connection.src_id];
    connection.src_port_type =
        static_cast<IO::flags_t>(id_connection.src_direction);
    connection.src_port = static_cast<size_t>(id_connection.src_port);
    connection.dest = block_cache[id_connection.dest_id];
    connection.dest_port = static_cast<size_t>(id_connection.dest_port);
    connection_events.emplace_back(Event::Type::IO_LINK_INSERT_EVENT);
    connection_events.back().setParam("connection", std::any(connection));
  }
  userprefs.endGroup();  // Connections
  event_manager->postEvent(connection_events);
}

void MainWindow::loadSettings()
{
  QSettings userprefs;
  userprefs.beginGroup("Workspaces");
  auto* load_settings_dialog = new QInputDialog(this);
  load_settings_dialog->setInputMode(QInputDialog::TextInput);
  load_settings_dialog->setComboBoxEditable(false);
  load_settings_dialog->setComboBoxItems(userprefs.childKeys());
  load_settings_dialog->setLabelText("Profile");
  load_settings_dialog->setOkButtonText("Load");
  load_settings_dialog->exec();

  if (load_settings_dialog->result() == QDialog::Rejected) {
    userprefs.endGroup();
    return;
  }

  const QString profile = load_settings_dialog->textValue();
  mdiArea->closeAllSubWindows();
  const auto workspace_filename = userprefs.value(profile).toString();
  QSettings workspaceprefs(workspace_filename, QSettings::IniFormat);
  std::unordered_map<int, IO::Block*> blocks;
  this->loadPeriodSettings(workspaceprefs);

  this->loadDAQSettings(workspaceprefs, blocks);

  this->loadWidgetSettings(workspaceprefs, blocks);

  this->loadConnectionSettings(workspaceprefs, blocks);

  userprefs.endGroup();  // workspaces
}

void MainWindow::saveSettings()
{
  QSettings userprefs;
  userprefs.beginGroup("settings");
  const auto workspace_dir_loc =
      userprefs
          .value(QString::fromStdString(
              std::string(UserPrefs::WORKSPACE_SAVE_LOCATION_KEY)))
          .toString();
  userprefs.endGroup();
  userprefs.beginGroup("Workspaces");
  auto* save_settings_dialog = new QInputDialog(this);
  save_settings_dialog->setInputMode(QInputDialog::TextInput);
  save_settings_dialog->setComboBoxEditable(true);
  save_settings_dialog->setComboBoxItems(userprefs.childKeys());
  save_settings_dialog->setLabelText("Profile");
  save_settings_dialog->setOkButtonText("Save");
  save_settings_dialog->exec();

  if (save_settings_dialog->result() == QDialog::Rejected) {
    userprefs.endGroup();
    return;
  }

  const QString profile_name = save_settings_dialog->textValue();
  if (userprefs.childGroups().contains(profile_name)) {
    userprefs.remove(profile_name);
  }
  const QString workspace_filename =
      workspace_dir_loc + "/" + profile_name + ".ws";
  userprefs.setValue(profile_name, workspace_filename);
  // userprefs.beginGroup(profile_name);
  QSettings workspaceprefs(workspace_filename, QSettings::IniFormat);
  workspaceprefs.clear();
  this->savePeriodSettings(workspaceprefs);

  this->saveDAQSettings(workspaceprefs);

  this->saveWidgetSettings(workspaceprefs);

  this->saveConnectionSettings(workspaceprefs);

  userprefs.endGroup();  // Workspaces
}

void MainWindow::resetSettings()
{
  mdiArea->closeAllSubWindows();
  // reset period to default
  Event::Object set_period_event(Event::Type::RT_PERIOD_EVENT);
  set_period_event.setParam("period", std::any(RT::OS::DEFAULT_PERIOD));
  event_manager->postEvent(&set_period_event);
}

void MainWindow::utilitiesMenuActivated(QAction* id)
{
  this->loadWidget(QCoreApplication::applicationDirPath() + QDir::separator()
                   + QString("rtxi_modules") + QDir::separator() + id->text());
}

void MainWindow::loadWidget(const QString& module_name)
{
  Widgets::Plugin* unused_pointer = nullptr;
  this->loadWidget(module_name, unused_pointer);
}

void MainWindow::loadWidget(const QString& module_name,
                            Widgets::Plugin*& rtxi_plugin_pointer)
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
  rtxi_plugin_pointer =
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
