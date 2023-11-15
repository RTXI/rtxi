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

#include "daq.hpp"
#include "dlplugin.hpp"
#include "event.hpp"
#include "widgets.hpp"

/*!
 * Objects contained within this namespace are responsible for providing
 *   Manager objects.
 */
namespace Workspace
{

/*
 * Returns a set of functions for generating plugin, component, and panel
 * classes for a core RTXI widgets
 *
 */
std::optional<Widgets::FactoryMethods> get_core_plugin_factory(
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
   *
   * \param library The name of the core library to load. For dynamically loaded
   *                widgets it is the location of the library in the filesystem.
   * \return Raw pointer to widgets plugin. The plugin is already registered
   * with the manager upon return of this function.
   */
  Widgets::Plugin* loadPlugin(const std::string& library);

  /*!
   * unloads plugin
   *
   * \param plugin Plugin pointer to remove from workspace manager registry
   */
  void unloadPlugin(Widgets::Plugin* plugin);

  /*!
   * Handles plugin loading/unloadin gevents from gui thread
   *
   * \param event The event to handle. The workspace manager only handles the
   * following event types: Event::Type::PLUGIN_INSERT_EVENT
   *                Event::Type::PLUGIN_REMOVE_EVENT
   *                Event::Type::DAQ_DEVICE_QUERY_EVENT
   */
  void receiveEvent(Event::Object* event) override;

  /*!
   * Checks whether plugin is registered.
   *
   * \param plugin Plugin pointer to check registration.
   *
   * \return True if registered, false otherwise.
   */
  bool isRegistered(const Widgets::Plugin* plugin);

  /*!
   * Get the list of all loaded devices in RTXI
   *
   * The workspace manager, upon instantiation, will search predefined places
   * for loadable DAQ device drivers. Once those drivers are successfully
   * loaded, the manager will store them in a registry. This returns the list of
   * all DAQ devices in the registry.
   *
   * \param driver The name of the driver associated with the devices
   * \return A vector of DAQ::Device pointers associated with the driver
   *
   * \sa DAQ::Device
   * \sa DAQ::Driver
   */
  std::vector<DAQ::Device*> getDevices(const std::string& driver);

  /*!
   * Get all devices in registry
   *
   * \return A vector of DAQ::Device pointers
   */
  std::vector<DAQ::Device*> getAllDevices();

  /*!
   * Save current workspace values and state settings
   *
   * \param profile_name The name to save the settings
   */
  void saveSettings(const QString& profile_name);

  /*!
   * Restore workspace values and state settings
   *
   * \param profile_name The name of the stored settings
   */
  void loadSettings(const QString& profile_name);

private:
  using driver_registry_entry = std::pair<std::string, DAQ::Driver*>;
  QString settings_prefix;
  void registerDriver(const std::string& driver_location);
  void unregisterDriver(const std::string& driver_location);

  [[nodiscard]] Widgets::Plugin* registerWidget(
      std::unique_ptr<Widgets::Plugin> widget);
  void unregisterWidget(Widgets::Plugin* plugin);

  void registerFactories(const std::string& widget_name,
                         Widgets::FactoryMethods fact);
  void unregisterFactories(const std::string& widget_name);
  Widgets::Plugin* loadCorePlugin(const std::string& library);

  std::unordered_map<std::string, std::vector<std::unique_ptr<Widgets::Plugin>>>
      rtxi_widgets_registry;
  std::vector<driver_registry_entry> m_driver_registry;
  std::unordered_map<std::string, Widgets::FactoryMethods>
      rtxi_factories_registry;
  Event::Manager* event_manager;
  std::unique_ptr<DLL::Loader> m_plugin_loader;
  std::unique_ptr<DLL::Loader> m_driver_loader;

  std::mutex m_widgets_mut;
  std::mutex m_drivers_mut;
};

}  // namespace Workspace

#endif  // WORKSPACE_H
