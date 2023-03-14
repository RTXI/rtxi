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

#include <QString>
#include <filesystem>
#include <typeinfo>

#include <dlfcn.h>
#include "plugin_tests.hpp"

TEST_F(PluginManagerTest, LoadandUnload)
{
  std::string libraryPath(std::filesystem::current_path().string());
  libraryPath += "libfakePlugin.so";
  Modules::Plugin* plugin= this->mod_manager->loadPlugin(libraryPath);
  ASSERT_NE(plugin, nullptr);
  this->mod_manager->unloadPlugin(plugin);
  ASSERT_FALSE(this->mod_manager->isRegistered(plugin));
}

TEST_F(PluginManagerTest, getLibrary)
{
  // TODO: Decouple Plugin::Object from Plugin::Manager and test getLibrary
  // function
  std::string libraryPath(std::filesystem::current_path().string());
  libraryPath += "libfakePlugin.so";
  Modules::Plugin* plugin= this->mod_manager->loadPlugin(libraryPath);
  EXPECT_EQ(libraryPath, plugin->getLibrary());
  this->mod_manager->unloadPlugin(plugin);
}


