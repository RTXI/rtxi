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
#include "rtxiConfig.h"
#include "system_control/system_control.h"
#include "userprefs/userprefs.h"

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
}

Workspace::Manager::~Manager()
{
  for (const auto& plugin_list : this->rtxi_modules_registry) {
    for (const auto& plugin : plugin_list.second) {
      this->event_manager->unregisterHandler(plugin.get());
    }
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
  std::string library_loc = library;
  Modules::Plugin* plugin_ptr = nullptr;
  // if module factory is already registered then all we have to do is run it
  if (this->rtxi_factories_registry.find(library_loc)
      != this->rtxi_factories_registry.end())
  {
    std::unique_ptr<Modules::Plugin> plugin =
        this->rtxi_factories_registry[library_loc].createPlugin(
            this->event_manager);
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
    // We try to load it from another location besides locally
    ERROR_MSG("Modules::Plugin::loadPlugin : could not load module locally");
    library_loc = RTXI_DEFAULT_PLUGIN_DIR + library;
    result = this->m_plugin_loader->load(library_loc.c_str());
  }
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

Modules::Plugin* Workspace::Manager::registerModule(
    std::unique_ptr<Modules::Plugin> module)
{
  std::unique_lock<std::mutex> lk(this->m_modules_mut);
  const std::string mod_name = module->getName();
  this->rtxi_modules_registry[mod_name].push_back(std::move(module));
  return this->rtxi_modules_registry[mod_name].back().get();
}

void Workspace::Manager::unregisterModule(Modules::Plugin* plugin)
{
  if (plugin == nullptr) {
    return;
  }
  std::unique_lock<std::mutex> lk(this->m_modules_mut);
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
    default:
      return;
  }
}
