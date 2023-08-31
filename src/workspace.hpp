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

#ifndef WORKSPACE_H
#define WORKSPACE_H

#include <utility>
#include "dlplugin.hpp"
#include "event.hpp"
#include "module.hpp"
#include "daq.hpp"

//! Internal Management Oriented Classes
/*!
 * Objects contained within this namespace are responsible for providing
 *   Manager objects.
 */
namespace Workspace
{

/*
 * Returns a set of functions for generating plugin, component, and panel
 * classes for a core RTXI modules
 *
 */
std::optional<Modules::FactoryMethods> get_core_plugin_factory(
    const std::string& plugin_name);

/*!
 * This class is responsible for managing Plugin device loading and unloading
 */
class Manager : public Event::Handler
{
public:
  Manager(const Manager&) = delete;
  Manager(Manager&&) = delete;
  Manager& operator=(const Manager&) = delete;
  Manager& operator=(Manager&&) = delete;
  explicit Manager(Event::Manager* ev_manager);
  ~Manager() override;

  /*!
   * loads plugin
   */
  Modules::Plugin* loadPlugin(const std::string& library);

  /*!
   * unloads plugin
   */
  void unloadPlugin(Modules::Plugin* plugin);

  /*!
   * Handles plugin loading/unloadin gevents from gui thread
   */
  void receiveEvent(Event::Object* event) override;

  /*!
   * Checks whether plugin is registered
   */
  bool isRegistered(const Modules::Plugin* plugin);

  std::vector<DAQ::Device*> getDevices(const std::string& driver);
  std::vector<DAQ::Device*> getAllDevices();

private:
  using driver_registry_entry = std::pair<std::string, DAQ::Driver*>;
  void registerDriver(const std::string& driver_location);
  void unregisterDriver(const std::string& driver_location);

  [[nodiscard]] Modules::Plugin* registerModule(
      std::unique_ptr<Modules::Plugin> module);
  void unregisterModule(Modules::Plugin* plugin);

  void registerFactories(const std::string& module_name,
                         Modules::FactoryMethods);
  void unregisterFactories(const std::string& module_name);
  Modules::Plugin* loadCorePlugin(const std::string& library);

  std::unordered_map<std::string, std::vector<std::unique_ptr<Modules::Plugin>>>
      rtxi_modules_registry;
  std::vector<std::pair<std::string, DAQ::Driver*>> m_driver_registry;
  std::unordered_map<std::string, Modules::FactoryMethods>
      rtxi_factories_registry;
  Event::Manager* event_manager;
  std::unique_ptr<DLL::Loader> m_plugin_loader;
  std::unique_ptr<DLL::Loader> m_driver_loader;

  std::mutex m_modules_mut;
  std::mutex m_drivers_mut;
};

}  // namespace Workspace

#endif  // WORKSPACE_H
