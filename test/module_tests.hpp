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
#ifndef MODULE_TESTS
#define MODULE_TESTS

#include <vector>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "event.hpp"
#include "io_tests.hpp"
#include "widgets.hpp"

enum TEST_PARAMETER_ID : size_t
{
  TESTINT = 0,
  TESTDOUBLE,
  TESTUINT,
  TESTCOMMENT,
  TESTSTATE,
};

std::vector<Widgets::Variable::Info> generateDefaultComponentVariables();

class mockModuleComponent : public Widgets::Component
{
public:
  mockModuleComponent()
      : Widgets::Component(nullptr,
                           std::string("testname"),
                           generateDefaultChannelList(),
                           generateDefaultComponentVariables())
  {
  }

  void execute() final
  {
    std::unique_lock lck(this->mut);
    this->executed = true;
    this->cond_var.notify_all();
  }

  bool wasExecuted()
  {
    std::unique_lock lck(this->mut);
    this->cond_var.wait(lck, [this]() { return this->executed; });
    return this->executed;
  }

private:
  bool executed = false;
  std::condition_variable cond_var;
  std::mutex mut;
};

class ModuleComponetTests : public ::testing::Test
{
protected:
  ModuleComponetTests() = default;
  ~ModuleComponetTests() override = default;

  // Widgets::Component component;
};

class ModulePanelTests : public ::testing::Test
{
protected:
  ModulePanelTests() = default;
  ~ModulePanelTests() override = default;
};

class ModulePluginTests : public ::testing::Test
{
protected:
  ModulePluginTests()
  {
    // this->main_window = new MainWindow();
    this->event_manager = std::make_unique<Event::Manager>();
    this->connector = std::make_unique<RT::Connector>();
    this->system = std::make_unique<RT::System>(this->event_manager.get(),
                                                this->connector.get());
    this->system->createTelemitryProcessor();
    auto component = std::make_unique<mockModuleComponent>();
    this->plugin = std::make_unique<Widgets::Plugin>(this->event_manager.get(),
                                                     "testname");
    component->setActive(/*act=*/true);
    this->component_ptr = component.get();
    this->plugin->attachComponent(std::move(component));
  }
  ~ModulePluginTests() override
  {
    // Event::Object shutdown_event(Event::Type::RT_SHUTDOWN_EVENT);
    // this->event_manager->postEvent(&shutdown_event);
  }

  std::unique_ptr<RT::Connector> connector;
  std::unique_ptr<Event::Manager> event_manager;
  std::unique_ptr<RT::System> system;
  // std::unique_ptr<Widgets::Manager> plugin_manager;

  std::unique_ptr<Widgets::Plugin> plugin;
  mockModuleComponent* component_ptr;
  //MainWindow* main_window = nullptr;
  // Widgets::Plugin plugin;
};

class ModuleManagerTests : public ::testing::Test
{
protected:
  ModuleManagerTests() {}
  ~ModuleManagerTests() override {}
};

#endif
