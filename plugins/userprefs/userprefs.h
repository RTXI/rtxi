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
#include "module.hpp"

namespace UserPrefs
{

constexpr std::string_view MODULE_NAME = "User Preferences";

class Plugin : public Modules::Plugin
{
public:
  explicit Plugin(Event::Manager* ev_manager);

};  // class Prefs

class Panel : public Modules::Panel
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
  QGroupBox* HDF = nullptr;
  QGroupBox* buttons = nullptr;

  QLineEdit* settingsDirEdit = nullptr;  // directory for settings files
  QLineEdit* dataDirEdit = nullptr;  // directory of most recent data file
  QLineEdit* HDFBufferEdit = nullptr;  // buffer size for HDF Data Recorder
};  // class Panel

std::unique_ptr<Modules::Plugin> createRTXIPlugin(Event::Manager* ev_manager);

Modules::Panel* createRTXIPanel(QMainWindow* main_window,
                                Event::Manager* ev_manager);
std::unique_ptr<Modules::Component> createRTXIComponent(
    Modules::Plugin* host_plugin);

Modules::FactoryMethods getFactories();

} // namespace UserPrefs
#endif /* USERPREFS */
