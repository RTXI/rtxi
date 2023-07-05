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

#ifndef PLUGIN_TESTS_H
#define PLUGIN_TESTS_H

#include <gtest/gtest.h>

#include "module.hpp"

class PluginObjectTest : public ::testing::Test
{
protected:
  PluginObjectTest() = default;
  ~PluginObjectTest() = default;
};

class PluginManagerTest : public ::testing::Test
{
protected:
  PluginManagerTest()
  {
    this->ev_manager = std::make_unique<Event::Manager>();
    this->mod_manager = std::make_unique<Modules::Manager>(ev_manager.get());
  }
  ~PluginManagerTest() = default;

  std::unique_ptr<Event::Manager> ev_manager;
  std::unique_ptr<Modules::Manager> mod_manager;
};

#endif
