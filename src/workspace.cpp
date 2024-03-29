/*
         The Real-Time eXperiment Interface (RTXI)
         Copyright (C) 2011 Georgia Institute of Technology, University of Utah,
   Will Cornell Medical College This program is free software: you can
   redistribute it and/or modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation, either version 3 of the
   License, or (at your option) any later version. This program is distributed
   in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
   the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
   See the GNU General Public License for more details. You should have received
   a copy of the GNU General Public License along with this program.  If not,
   see <http://www.gnu.org/licenses/>.
*/

#include <optional>
#include <variant>

#include "workspace.hpp"

#include "connector/connector.hpp"
#include "data_recorder/data_recorder.hpp"
#include "module_installer/rtxi_wizard.hpp"
#include "oscilloscope/oscilloscope.hpp"
#include "performance_measurement/performance_measurement.hpp"
#include "system_control/system_control.hpp"
#include "userprefs/userprefs.hpp"

std::optional<Widgets::FactoryMethods> Workspace::get_core_plugin_factory(
    const std::string& plugin_name)
{
  std::optional<Widgets::FactoryMethods> fact_methods;
  if (plugin_name == std::string(PerformanceMeasurement::MODULE_NAME)) {
    fact_methods = PerformanceMeasurement::getFactories();
  } else if (plugin_name == std::string(UserPrefs::MODULE_NAME)) {
    fact_methods = UserPrefs::getFactories();
  } else if (plugin_name == std::string(SystemControl::MODULE_NAME)) {
    fact_methods = SystemControl::getFactories();
  } else if (plugin_name == std::string(Connector::MODULE_NAME)) {
    fact_methods = Connector::getFactories();
  } else if (plugin_name == std::string(Oscilloscope::MODULE_NAME)) {
    fact_methods = Oscilloscope::getFactories();
  } else if (plugin_name == std::string(DataRecorder::MODULE_NAME)) {
    fact_methods = DataRecorder::getFactories();
  } else if (plugin_name == std::string(RTXIWizard::MODULE_NAME)) {
    fact_methods = RTXIWizard::getFactories();
  } else {
    return fact_methods;
  }
  return fact_methods;
}

Workspace::Manager::Manager(Event::Manager* ev_manager)
    : event_manager(ev_manager)
{
  this->event_manager->registerHandler(this);
  this->m_plugin_loader = std::make_unique<DLL::Loader>();
  this->m_driver_loader = std::make_unique<DLL::Loader>();
  const QDir bin_dir = QCoreApplication::applicationDirPath();
  const std::string nidaq_driver_name = "librtxinidaqdriver.so";
  if (bin_dir.exists(QString::fromStdString(nidaq_driver_name))) {
    this->registerDriver(bin_dir.path().toStdString() + std::string("/")
                         + nidaq_driver_name);
  }
}

Workspace::Manager::~Manager()
{
  for (const auto& plugin_list : this->rtxi_widgets_registry) {
    for (const auto& plugin : plugin_list.second) {
      this->event_manager->unregisterHandler(plugin.get());
    }
  }
  const std::vector<DAQ::Device*> devices = getAllDevices();
  std::vector<Event::Object> unregister_device_events(
      devices.size(), Event::Object(Event::Type::RT_DEVICE_REMOVE_EVENT));
  for (size_t i = 0; i < devices.size(); ++i) {
    unregister_device_events[i].setParam(
        "device", std::any(static_cast<RT::Device*>(devices[i])));
  }
  this->event_manager->postEvent(unregister_device_events);
  // we should unload all drivers
  void (*unloadDriversFunc)() = nullptr;
  for (auto& driver : m_driver_registry) {
    unloadDriversFunc = this->m_driver_loader->dlsym<void (*)()>(
        driver.first.c_str(), "deleteRTXIDAQDriver");
    unloadDriversFunc();
  }
  this->event_manager->unregisterHandler(this);
}

bool Workspace::Manager::isRegistered(const Widgets::Plugin* plugin)
{
  std::vector<std::unique_ptr<Widgets::Plugin>>::iterator start_iter;
  std::vector<std::unique_ptr<Widgets::Plugin>>::iterator end_iter;
  bool registered = false;
  for (auto& widgets_list : rtxi_widgets_registry) {
    start_iter = widgets_list.second.begin();
    end_iter = widgets_list.second.end();
    registered = std::any_of(
        start_iter,
        end_iter,
        [plugin](const std::unique_ptr<Widgets::Plugin>& temp_plugin)
        { return plugin == temp_plugin.get(); });
    if (registered) {
      break;
    }
  }
  return registered;
}

std::vector<DAQ::Device*> Workspace::Manager::getDevices(
    const std::string& driver)
{
  auto iter = std::find_if(m_driver_registry.begin(),
                           m_driver_registry.end(),
                           [&](const driver_registry_entry& entry)
                           { return entry.second->getDriverName() == driver; });
  if (iter == m_driver_registry.end()) {
    return {};
  }
  return iter->second->getDevices();
}

std::vector<DAQ::Device*> Workspace::Manager::getAllDevices()
{
  std::vector<DAQ::Device*> devices;
  std::vector<DAQ::Device*> temp_driver_devices;
  for (auto& entry : this->m_driver_registry) {
    temp_driver_devices = entry.second->getDevices();
    devices.insert(
        devices.end(), temp_driver_devices.begin(), temp_driver_devices.end());
  }
  return devices;
}

template<class... Ts>
struct overload : Ts...
{
  using Ts::operator()...;
};
template<class... Ts>
overload(Ts...) -> overload<Ts...>;
void Workspace::Manager::saveSettings(const QString& profile_name)
{
  QSettings settings(settings_prefix + profile_name, QSettings::IniFormat);
  settings.beginGroup("widgets");
  QString widget_name;
  int widget_count = 0;
  for (const auto& entry : this->rtxi_widgets_registry) {
    widget_name = QString::fromStdString(entry.first);
    settings.beginGroup(widget_name);
    for (const auto& plugin : entry.second) {
      settings.beginGroup(QString::number(widget_count));
      for (const auto& param_info : plugin->getComponentParametersInfo()) {
        settings.setValue(
            QString::fromStdString(param_info.name),
            std::visit(overload {[](const int64_t& val) -> QString
                                 { return QString::number(val); },
                                 [](const double& val) -> QString
                                 { return QString::number(val); },
                                 [](const uint64_t& val) -> QString
                                 { return QString::number(val); },
                                 [](const std::string& val) -> QString
                                 { return QString::fromStdString(val); },
                                 [](const RT::State::state_t& val) -> QString {
                                   return QString::number(
                                       static_cast<int8_t>(val));
                                 }},
                       param_info.value));
      }
      settings.endGroup();
      widget_count++;
    }
    settings.endGroup();
  }
  settings.endGroup();
}

void Workspace::Manager::loadSettings(const QString& profile_name)
{
  QSettings settings(profile_name, QSettings::IniFormat);
}

Widgets::Plugin* Workspace::Manager::loadCorePlugin(const std::string& library)
{
  Widgets::Plugin* plugin_ptr = nullptr;
  std::optional<Widgets::FactoryMethods> fact_methods =
      Workspace::get_core_plugin_factory(library);
  if (!fact_methods.has_value()) {
    return nullptr;
  }
  std::unique_ptr<Widgets::Plugin> plugin;
  this->registerFactories(library, *fact_methods);
  plugin = this->rtxi_factories_registry[library].createPlugin(event_manager);
  plugin_ptr = this->registerWidget(std::move(plugin));
  return plugin_ptr;
}

// TODO: extract plugin dynamic loading to another class
Widgets::Plugin* Workspace::Manager::loadPlugin(const std::string& library)
{
  const std::string& library_loc = library;
  Widgets::Plugin* plugin_ptr = nullptr;
  // if widget factory is already registered then all we have to do is run it
  if (this->rtxi_factories_registry.find(library_loc)
      != this->rtxi_factories_registry.end())
  {
    std::unique_ptr<Widgets::Plugin> plugin =
        this->rtxi_factories_registry[library_loc].createPlugin(
            this->event_manager);
    plugin_ptr = this->registerWidget(std::move(plugin));
    plugin_ptr->attachComponent(
        this->rtxi_factories_registry[library_loc].createComponent(plugin_ptr));
    return plugin_ptr;
  }

  // If it is just a core plugin then handle that elsewhere and return
  plugin_ptr = this->loadCorePlugin(library_loc);
  if (plugin_ptr != nullptr) {
    return plugin_ptr;
  }

  if (this->m_plugin_loader->load(library_loc.c_str()) != 0) {
    ERROR_MSG("Plugin::load : failed to load {}", library_loc.c_str());
    return nullptr;
  }

  auto gen_fact_methods =
      this->m_plugin_loader->dlsym<Widgets::FactoryMethods* (*)()>(
          library_loc.c_str(), "getFactories");

  if (gen_fact_methods == nullptr) {
    ERROR_MSG("Plugin::load : failed to retrieve getFactories symbol");
    // If we got here it means we loaded the lirbary but not the symbol.
    // Let's just unload the library and exit before we regret it.
    this->m_plugin_loader->unload(library_loc.c_str());
    return nullptr;
  }

  Widgets::FactoryMethods* fact_methods = gen_fact_methods();
  this->rtxi_factories_registry[library] = *fact_methods;
  std::unique_ptr<Widgets::Plugin> plugin =
      fact_methods->createPlugin(this->event_manager);
  plugin->setLibrary(library);
  if (plugin == nullptr) {
    ERROR_MSG("Plugin::load : failed to create plugin from library {} ",
              library);
    this->m_plugin_loader->unload(library_loc.c_str());
    return nullptr;
  }
  std::unique_ptr<Widgets::Component> component;
  try {
    component = fact_methods->createComponent(plugin.get());
  } catch (const std::invalid_argument& e) {
    this->m_plugin_loader->unload(library_loc.c_str());
    return nullptr;
  }
  plugin->attachComponent(std::move(component));
  // if (plugin->magic_number != Plugin::Object::MAGIC_NUMBER) {
  //   ERROR_MSG(
  //       "Plugin::load : the pointer returned from {}::createRTXIPlugin()
  //       isn't " "a valid Plugin::Object *.\n",
  //       library.toStdString().c_str());
  //   dlclose(handle);
  //   return 0;
  // }
  plugin_ptr = this->registerWidget(std::move(plugin));

  return plugin_ptr;
}

void Workspace::Manager::unloadPlugin(Widgets::Plugin* plugin)
{
  const std::string library = plugin->getName();
  this->unregisterWidget(plugin);
  if (this->rtxi_widgets_registry[library].empty()) {
    this->m_plugin_loader->unload(library.c_str());
    this->unregisterFactories(library);
  }
}

void Workspace::Manager::registerDriver(const std::string& driver_location)
{
  auto iter = std::find_if(m_driver_registry.begin(),
                           m_driver_registry.end(),
                           [&](const driver_registry_entry& entry)
                           { return entry.first == driver_location; });
  if (iter != this->m_driver_registry.end()) {
    return;
  }
  if (this->m_driver_loader->load(driver_location.c_str()) < 0) {
    return;
  }
  auto getDriver = this->m_driver_loader->dlsym<DAQ::Driver* (*)()>(
      driver_location.c_str(), "getRTXIDAQDriver");
  if (getDriver == nullptr) {
    ERROR_MSG(
        "Workspace::Manager::registerDriver : Unable to load dynamic library "
        "file {}",
        driver_location);
    return;
  }
  DAQ::Driver* driver = getDriver();
  if (driver == nullptr) {
    ERROR_MSG(
        "Workspace::Manager::registerDriver : Unable to load driver from "
        "library {}",
        driver_location);
    return;
  }
  this->m_driver_registry.emplace_back(driver_location, driver);
  std::vector<Event::Object> plug_device_events;
  for (auto* device : driver->getDevices()) {
    plug_device_events.emplace_back(Event::Type::RT_DEVICE_INSERT_EVENT);
    plug_device_events.back().setParam(
        "device", std::any(static_cast<RT::Device*>(device)));
  }
  this->event_manager->postEvent(plug_device_events);
}

void Workspace::Manager::unregisterDriver(const std::string& driver_location)
{
  auto iter = std::find_if(m_driver_registry.begin(),
                           m_driver_registry.end(),
                           [&](const driver_registry_entry& entry)
                           { return entry.first == driver_location; });

  if (iter == this->m_driver_registry.end()) {
    return;
  }
  std::vector<Event::Object> unplug_device_events;
  for (auto* device : iter->second->getDevices()) {
    unplug_device_events.emplace_back(Event::Type::RT_DEVICE_REMOVE_EVENT);
    unplug_device_events.back().setParam(
        "device", std::any(static_cast<RT::Device*>(device)));
  }
  this->event_manager->postEvent(unplug_device_events);
  this->m_driver_registry.erase(iter);
  this->m_driver_loader->unload(driver_location.c_str());
}

Widgets::Plugin* Workspace::Manager::registerWidget(
    std::unique_ptr<Widgets::Plugin> widget)
{
  const std::unique_lock<std::mutex> lk(this->m_widgets_mut);
  const std::string mod_name = widget->getName();
  this->rtxi_widgets_registry[mod_name].push_back(std::move(widget));
  return this->rtxi_widgets_registry[mod_name].back().get();
}

void Workspace::Manager::unregisterWidget(Widgets::Plugin* plugin)
{
  if (plugin == nullptr) {
    return;
  }
  const std::unique_lock<std::mutex> lk(this->m_widgets_mut);
  const std::string plugin_name = plugin->getName();
  auto start_iter = this->rtxi_widgets_registry[plugin_name].begin();
  auto end_iter = this->rtxi_widgets_registry[plugin_name].end();
  auto loc =
      std::find_if(start_iter,
                   end_iter,
                   [plugin](const std::unique_ptr<Widgets::Plugin>& widget)
                   { return plugin == widget.get(); });
  if (loc == end_iter) {
    return;
  }
  // this->m_plugin_loader->unload(plugin->getLibrary().c_str());
  this->rtxi_widgets_registry[plugin_name].erase(loc);
}

void Workspace::Manager::registerFactories(const std::string& widget_name,
                                           Widgets::FactoryMethods fact)
{
  this->rtxi_factories_registry[widget_name] = fact;
}

void Workspace::Manager::unregisterFactories(const std::string& widget_name)
{
  if (this->rtxi_factories_registry.find(widget_name)
      != this->rtxi_factories_registry.end())
  {
    this->rtxi_factories_registry.erase(widget_name);
  }
}

void Workspace::Manager::receiveEvent(Event::Object* event)
{
  std::string plugin_name;
  Widgets::Plugin* plugin_ptr = nullptr;
  switch (event->getType()) {
    case Event::Type::PLUGIN_REMOVE_EVENT:
      plugin_ptr =
          std::any_cast<Widgets::Plugin*>(event->getParam("pluginPointer"));
      this->unloadPlugin(plugin_ptr);
      break;
    case Event::Type::PLUGIN_INSERT_EVENT:
      plugin_name = std::any_cast<std::string>(event->getParam("pluginName"));
      plugin_ptr = this->loadPlugin(plugin_name);
      if (plugin_ptr != nullptr) {
        event->setParam("status", std::any(std::string("success")));
        event->setParam(
            "createRTXIPanel",
            std::any(this->rtxi_factories_registry[plugin_name].createPanel));
        event->setParam("pluginPointer", std::any(plugin_ptr));
      } else {
        event->setParam("status", std::any(std::string("failure")));
      }
      break;
    case Event::Type::DAQ_DEVICE_QUERY_EVENT:
      event->setParam("devices", std::any(this->getAllDevices()));
      break;
    default:
      return;
  }
}
