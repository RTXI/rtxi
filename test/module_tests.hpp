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
#ifndef MODULE_TESTS
#define MODULE_TESTS

#include <vector>

#include <gtest/gtest.h>
#include <gmock/gmock.h>


#include "event.hpp"
#include "main_window.hpp"
#include "module.hpp"
#include "io_tests.hpp"

std::vector<Modules::Variable::Info> generateDefaultComponentVariables();

class mockModuleComponent : public Modules::Component
{
public:
  mockModuleComponent() : Modules::Component(
    nullptr, 
    std::string("testcomponent"), 
    generateDefaultChannelList(), 
    generateDefaultComponentVariables()) {}

  MOCK_METHOD(void, execute, (), (override));
};

class ModuleComponetTests : public ::testing::Test
{
protected:
  ModuleComponetTests() {

  }
  ~ModuleComponetTests() {}

  //Modules::Component componnent;
};

class ModulePanelTests : public ::testing::Test
{
protected:
  ModulePanelTests() {}
  ~ModulePanelTests() {}
};

class ModulePluginTests : public ::testing::Test
{
protected:
  ModulePluginTests() 
  {
    //this->main_window = new MainWindow();
    this->event_manager = std::make_unique<Event::Manager>();
    this->connector = std::make_unique<RT::Connector>();
    this->system = std::make_unique<RT::System>(this->event_manager.get(), this->connector.get());

    this->plugin = std::make_unique<Modules::Plugin>(this->event_manager.get(), this->main_window, "testname");
  
  }

  std::unique_ptr<RT::Connector> connector;
  std::unique_ptr<Event::Manager> event_manager;
  std::unique_ptr<RT::System> system;

  std::unique_ptr<Modules::Plugin> plugin;
  MainWindow* main_window = nullptr;
  //Modules::Plugin plugin;
};

class ModuleManagerTests : public ::testing::Test
{
protected:
  ModuleManagerTests() {}
  ~ModuleManagerTests() {}
};

#endif