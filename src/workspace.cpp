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

#include "workspace.hpp"

#include "connector/connector.h"
#include "data_recorder/data_recorder.h"
#include "oscilloscope/oscilloscope.h"
#include "performance_measurement/performance_measurement.hpp"
#include "system_control/system_control.h"
#include "userprefs/userprefs.h"
#include "module_installer/rtxi_wizard.h"
#include "rtxiConfig.h"

std::optional<Modules::FactoryMethods> Workspace::get_core_plugin_factory(
    const std::string& plugin_name)
{
  std::optional<Modules::FactoryMethods> fact_methods;
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
  QDir bin_dir = QCoreApplication::applicationDirPath();
  this->registerDriver(bin_dir.path().toStdString()+std::string("/librtxinidaqdriver.so"));
}

Workspace::Manager::~Manager()
{
  for (const auto& plugin_list : this->rtxi_modules_registry) {
    for (const auto& plugin : plugin_list.second) {
      this->event_manager->unregisterHandler(plugin.get());
    }
  }
  const std::vector<DAQ::Device*> devices = getAllDevices();
  std::vector<Event::Object> unregister_device_events(devices.size(), Event::Object(Event::Type::RT_DEVICE_REMOVE_EVENT));
  for(size_t i=0; i<devices.size(); ++i){
    unregister_device_events[i].setParam("device", std::any(static_cast<RT::Device*>(devices[i])));
  }
  this->event_manager->postEvent(unregister_device_events);
  // we should unload all drivers 
  void (*unloadDriversFunc )() = nullptr;
  for(auto& driver : m_driver_registry){
    unloadDriversFunc = this->m_driver_loader->dlsym<void (*)()>(driver.first.c_str(), "deleteRTXIDAQDriver");
    unloadDriversFunc();
  }
  this->event_manager->unregisterHandler(this);
}

bool Workspace::Manager::isRegistered(const Modules::Plugin* plugin)
{
  const std::string plugin_name = plugin->getName();
  auto start_iter = this->rtxi_modules_registry[plugin_name].begin();
  auto end_iter = this->rtxi_modules_registry[plugin_name].end();
  return std::any_of(start_iter,
                     end_iter,
                     [plugin](const std::unique_ptr<Modules::Plugin>& module)
                     { return plugin == module.get(); });
}

std::vector<DAQ::Device*> Workspace::Manager::getDevices(const std::string& driver)
{
  auto iter = std::find_if(m_driver_registry.begin(),
                           m_driver_registry.end(),
                           [&](const driver_registry_entry& entry){ return entry.second->getDriverName() == driver;}); 
  if (iter == m_driver_registry.end()) {
    return {};
  }
  return iter->second->getDevices();
}

std::vector<DAQ::Device*> Workspace::Manager::getAllDevices()
{
  std::vector<DAQ::Device*> devices;
  std::vector<DAQ::Device*> temp_driver_devices;
  for(auto& entry : this->m_driver_registry){
    temp_driver_devices = entry.second->getDevices();
    devices.insert(devices.end(),
                   temp_driver_devices.begin(),
                   temp_driver_devices.end());
  }
  return devices;
} 

Modules::Plugin* Workspace::Manager::loadCorePlugin(const std::string& library)
{
  Modules::Plugin* plugin_ptr = nullptr;
  std::optional<Modules::FactoryMethods> fact_methods =
      Workspace::get_core_plugin_factory(library);
  if (!fact_methods.has_value()) {
    return nullptr;
  }
  std::unique_ptr<Modules::Plugin> plugin;
  this->registerFactories(library, *fact_methods);
  plugin = this->rtxi_factories_registry[library].createPlugin(event_manager);
  plugin_ptr = this->registerModule(std::move(plugin));
  return plugin_ptr;
}

// TODO: extract plugin dynamic loading to another class
Modules::Plugin* Workspace::Manager::loadPlugin(const std::string& library)
{
  const std::string& library_loc = library;
  Modules::Plugin* plugin_ptr = nullptr;
  // if module factory is already registered then all we have to do is run it
  if (this->rtxi_factories_registry.find(library_loc)
      != this->rtxi_factories_registry.end())
  {
    std::unique_ptr<Modules::Plugin> plugin =
        this->rtxi_factories_registry[library_loc].createPlugin(
            this->event_manager);
    std::unique_ptr<Modules::Component> component =
        this->rtxi_factories_registry[library_loc].createComponent(
            plugin.get());
    plugin->attachComponent(std::move(component));
    plugin_ptr = this->registerModule(std::move(plugin));
    return plugin_ptr;
  }

  // If it is just a core plugin then handle that elsewhere and return
  plugin_ptr = this->loadCorePlugin(library_loc);
  if (plugin_ptr != nullptr) {
    return plugin_ptr;
  }

  int result = this->m_plugin_loader->load(library_loc.c_str());
  if (result != 0) {
    ERROR_MSG("Plugin::load : failed to load {}", library_loc.c_str());
    return nullptr;
  }

  auto gen_fact_methods =
      this->m_plugin_loader->dlsym<Modules::FactoryMethods* (*)()>(
          library_loc.c_str(), "getFactories");

  if (gen_fact_methods == nullptr) {
    ERROR_MSG("Plugin::load : failed to retreive getFactories symbol");
    // If we got here it means we loaded the lirbary but not the symbol.
    // Let's just unload the library and exit before we regret it.
    this->m_plugin_loader->unload(library_loc.c_str());
    return nullptr;
  }

  Modules::FactoryMethods* fact_methods = gen_fact_methods();
  this->rtxi_factories_registry[library] = *fact_methods;
  std::unique_ptr<Modules::Plugin> plugin =
      fact_methods->createPlugin(this->event_manager);
  if (plugin == nullptr) {
    ERROR_MSG("Plugin::load : failed to create plugin from library {} ",
              library);
    this->m_plugin_loader->unload(library_loc.c_str());
    return nullptr;
  }
  std::unique_ptr<Modules::Component> component =
      fact_methods->createComponent(plugin.get());
  plugin->attachComponent(std::move(component));
  // if (plugin->magic_number != Plugin::Object::MAGIC_NUMBER) {
  //   ERROR_MSG(
  //       "Plugin::load : the pointer returned from {}::createRTXIPlugin()
  //       isn't " "a valid Plugin::Object *.\n",
  //       library.toStdString().c_str());
  //   dlclose(handle);
  //   return 0;
  // }
  plugin_ptr = this->registerModule(std::move(plugin));

  return plugin_ptr;
}

void Workspace::Manager::unloadPlugin(Modules::Plugin* plugin)
{
  const std::string library = plugin->getName();
  this->unregisterModule(plugin);
  if (this->rtxi_modules_registry[library].empty()) {
    this->unregisterFactories(library);
  }
}

void Workspace::Manager::registerDriver(const std::string& driver_location)
{
  auto iter = std::find_if(m_driver_registry.begin(),
                           m_driver_registry.end(),
                           [&](const driver_registry_entry& entry){ return entry.first == driver_location;}); 
  if(iter != this->m_driver_registry.end()){ return; }
  int errcode = this->m_driver_loader->load(driver_location.c_str());
  if(errcode < 0){ return; }
  auto getDriver = this->m_driver_loader->dlsym<DAQ::Driver* (*)()>(driver_location.c_str(), "getRTXIDAQDriver");
  if(getDriver == nullptr) {
    ERROR_MSG("Workspace::Manager::registerDriver : Unable to load dynamic library file {}", driver_location);
    return;
  }
  DAQ::Driver* driver = getDriver();
  if(driver == nullptr) {
    ERROR_MSG("Workspace::Manager::registerDriver : Unable to load driver from library {}", driver_location);
    return;
  }
  this->m_driver_registry.emplace_back(driver_location, driver);
  std::vector<Event::Object> plug_device_events;
  for(auto* device: driver->getDevices()){
    plug_device_events.emplace_back(Event::Type::RT_DEVICE_INSERT_EVENT);
    plug_device_events.back().setParam("device", std::any(static_cast<RT::Device*>(device)));
  }
  this->event_manager->postEvent(plug_device_events);
}

void Workspace::Manager::unregisterDriver(const std::string& driver_location)
{
  auto iter = std::find_if(m_driver_registry.begin(),
                           m_driver_registry.end(),
                           [&](const driver_registry_entry& entry){ return entry.first == driver_location;}); 

  if(iter == this->m_driver_registry.end()){
    return;
  }
  std::vector<Event::Object> unplug_device_events;
  for(auto *device : iter->second->getDevices()){
    unplug_device_events.emplace_back(Event::Type::RT_DEVICE_REMOVE_EVENT);
    unplug_device_events.back().setParam("device", std::any(static_cast<RT::Device*>(device)));
  }
  this->event_manager->postEvent(unplug_device_events);
  this->m_driver_registry.erase(iter);
  this->m_driver_loader->unload(driver_location.c_str());
}

Modules::Plugin* Workspace::Manager::registerModule(
    std::unique_ptr<Modules::Plugin> module)
{
  const std::unique_lock<std::mutex> lk(this->m_modules_mut);
  const std::string mod_name = module->getName();
  this->rtxi_modules_registry[mod_name].push_back(std::move(module));
  return this->rtxi_modules_registry[mod_name].back().get();
}

void Workspace::Manager::unregisterModule(Modules::Plugin* plugin)
{
  if (plugin == nullptr) {
    return;
  }
  const std::unique_lock<std::mutex> lk(this->m_modules_mut);
  const std::string plugin_name = plugin->getName();
  auto start_iter = this->rtxi_modules_registry[plugin_name].begin();
  auto end_iter = this->rtxi_modules_registry[plugin_name].end();
  auto loc =
      std::find_if(start_iter,
                   end_iter,
                   [plugin](const std::unique_ptr<Modules::Plugin>& module)
                   { return plugin == module.get(); });
  if (loc == end_iter) {
    return;
  }
  this->m_plugin_loader->unload(plugin->getLibrary().c_str());
  this->rtxi_modules_registry[plugin_name].erase(loc);
}

void Workspace::Manager::registerFactories(const std::string& module_name,
                                           Modules::FactoryMethods fact)
{
  this->rtxi_factories_registry[module_name] = fact;
}

void Workspace::Manager::unregisterFactories(const std::string& module_name)
{
  if (this->rtxi_factories_registry.find(module_name)
      != this->rtxi_factories_registry.end())
  {
    this->rtxi_factories_registry.erase(module_name);
  }
}

void Workspace::Manager::receiveEvent(Event::Object* event)
{
  std::string plugin_name;
  Modules::Plugin* plugin_ptr = nullptr;
  switch (event->getType()) {
    case Event::Type::PLUGIN_REMOVE_EVENT:
      plugin_ptr =
          std::any_cast<Modules::Plugin*>(event->getParam("pluginPointer"));
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
    case Event::Type::DAQ_DEVICE_QUERY_EVENT : 
      event->setParam("devices", std::any(this->getAllDevices()));
      break;
    default:
      return;
  }
}
