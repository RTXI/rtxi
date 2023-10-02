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

#include "plugin_tests.hpp"

#include <dlfcn.h>

TEST_F(PluginManagerTest, LoadandUnload)
{
  std::filesystem::path libraryPath =
      std::filesystem::canonical("/proc/self/exe").parent_path();
  std::filesystem::path library(libraryPath / "libfakePlugin.so");
  Widgets::Plugin* plugin = this->mod_manager->loadPlugin(library.string());
  ASSERT_NE(plugin, nullptr);
  this->mod_manager->unloadPlugin(plugin);
  ASSERT_FALSE(this->mod_manager->isRegistered(plugin));
}

TEST_F(PluginManagerTest, getLibrary)
{
  // TODO: Decouple Plugin::Object from Plugin::Manager and test getLibrary
  // function
  std::filesystem::path libraryPath =
      std::filesystem::canonical("/proc/self/exe").parent_path();
  std::filesystem::path library(libraryPath / "libfakePlugin.so");
  Widgets::Plugin* plugin = this->mod_manager->loadPlugin(library.string());
  EXPECT_EQ(library, plugin->getLibrary());
  this->mod_manager->unloadPlugin(plugin);
}
