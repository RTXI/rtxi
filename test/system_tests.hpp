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

#ifndef SYSTEM_TESTS_H
#define SYSTEM_TESTS_H

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "rt.hpp"
//#include <event.h>

// Define all fixtures for testing purposes
class SystemTest : public ::testing::Test
{
protected:
  SystemTest()
  {
    this->event_manager = std::make_unique<Event::Manager>();
    this->io_connector = std::make_unique<IO::Connector>();
    this->system = std::make_unique<RT::System>(event_manager.get(),
                                                io_connector.get());
  }
  std::unique_ptr<Event::Manager> event_manager;
  std::unique_ptr<IO::Connector> io_connector;
  std::unique_ptr<RT::System> system;
};

class MockRTDevice : public RT::Device
{
public:
  MOCK_METHOD(void, read, (), (override));
  MOCK_METHOD(void, write, (), (override));
};

class MockRTThread : public RT::Thread
{
public:
  MOCK_METHOD(unsigned long, getPriority, (), (const));
  MOCK_METHOD(void, execute, (), (override));
};

#endif
