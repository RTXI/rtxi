/*
         The Real-Time eXperiment Interface (RTXI)
         Copyright (C) 2011 Georgia Institute of Technology, University of Utah,
   Weill Cornell Medical College

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
#include <fakePlugin.h>
#include <plugin_tests.h>

TEST_F(PluginManagerTest, getInstance)
{
  manager = Plugin::Manager::getInstance();
  ASSERT_EQ(manager, Plugin::Manager::getInstance());
  ASSERT_EQ(manager, manager->getInstance());
}

TEST_F(PluginManagerTest, load)
{
  manager = Plugin::Manager::getInstance();
  QString libraryPath(std::filesystem::current_path().string().c_str());
  libraryPath += "/.libs/fakePlugin.so";
  Plugin::Object* testobject = new Plugin::Object();
  ASSERT_EQ(typeid(testobject).name(),
            typeid(manager->load(libraryPath)).name());
  delete testobject;
}

TEST_F(PluginManagerTest, unload)
{
  manager = Plugin::Manager::getInstance();
  Plugin::Object* plugin;
  QString libraryPath(std::filesystem::current_path().string().c_str());
  libraryPath += "/.libs/fakePlugin.so";
  plugin = manager->load(libraryPath);
  manager->unload(plugin);
  delete plugin;
  // Plugin::Manager uses QEvents from QT to tell itself that it needs to unload
  // plugins... why?
  // TODO: eliminate the need to use QEvents in this instance
  // TODO: How can I tell if a single plugin has been unloaded? answer: you
  // can't *face palm*
}

TEST_F(PluginManagerTest, unloadAll)
{
  manager = Plugin::Manager::getInstance();
  Plugin::Object** plugins = new Plugin::Object*[5];
  QString libraryPath(std::filesystem::current_path().string().c_str());
  libraryPath += "/.libs/fakePlugin.so";
  for (int i = 0; i < 5; ++i) {
    plugins[i] = manager->load(libraryPath);
  }
  manager->unloadAll();
  delete[] plugins;
}

TEST_F(PluginManagerTest, foreachPlugin)
{
  manager = Plugin::Manager::getInstance();
  // TODO: create a test plugin for testing foreachPlugin function in
  // Plugin::Manager
}

TEST_F(PluginObjectTest, getLibrary)
{
  // TODO: Decouple Plugin::Object from Plugin::Manager and test getLibrary
  // function
  Plugin::Manager* manager = Plugin::Manager::getInstance();
  QString libraryPath(std::filesystem::current_path().string().c_str());
  libraryPath += "/.libs/fakePlugin.so";
  object = manager->load(libraryPath);
  EXPECT_EQ(libraryPath.toStdString(), object->getLibrary());
  manager->unload(object);
  delete object;
}

TEST_F(PluginObjectTest, unload)
{
  // Its impossible to test this. No interface provided in order to check
  // loading and unloading status. Mainly tied to the manager class.
  // TODO: Decouple Plugin::Object from Plugin::Manager and test unload function
}
