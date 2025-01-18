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

#ifndef USERPREFS_H
#define USERPREFS_H

#include <QSettings>

#include "widgets.hpp"

namespace UserPrefs
{

constexpr std::string_view MODULE_NAME = "User Preferences";
constexpr std::string_view WORKSPACE_SAVE_LOCATION_KEY = "wokrspace_dir";
constexpr std::string_view HDF5_SAVE_LOCATION_KEY = "hdf5_dir";

class Plugin : public Widgets::Plugin
{
public:
  explicit Plugin(Event::Manager* ev_manager);

};  // class Prefs

class Panel : public Widgets::Panel
{
  Q_OBJECT

public:
  Panel(QMainWindow* mwindow, Event::Manager* ev_manager);

public slots:
  void apply();  // save and close
  void reset();  // reset to defaults

  void chooseSettingsDir();
  void chooseDataDir();

private:
  QLabel* status = nullptr;
  QSettings userprefs;

  QGroupBox* dirGroup = nullptr;
  QGroupBox* buttons = nullptr;

  QLineEdit* settingsDirEdit = nullptr;  // directory for settings files
  QLineEdit* dataDirEdit = nullptr;  // directory of most recent data file
};  // class Panel

std::unique_ptr<Widgets::Plugin> createRTXIPlugin(Event::Manager* ev_manager);

Widgets::Panel* createRTXIPanel(QMainWindow* main_window,
                                Event::Manager* ev_manager);
std::unique_ptr<Widgets::Component> createRTXIComponent(
    Widgets::Plugin* host_plugin);

Widgets::FactoryMethods getFactories();

}  // namespace UserPrefs
#endif /* USERPREFS */
