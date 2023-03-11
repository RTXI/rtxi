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

#ifndef SYSTEM_TESTS_H
#define SYSTEM_TESTS_H

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "rt.hpp"
// #include <event.h>

// Define all fixtures for testing purposes
class SystemTest : public ::testing::Test
{
protected:
  SystemTest()
  {
    this->event_manager = std::make_unique<Event::Manager>();
    this->rt_connector = std::make_unique<RT::Connector>();
    this->system =
        std::make_unique<RT::System>(event_manager.get(), rt_connector.get());
  }
  std::unique_ptr<Event::Manager> event_manager;
  std::unique_ptr<RT::Connector> rt_connector;
  std::unique_ptr<RT::System> system;
};

class MockRTDevice : public RT::Device
{
public:
  MockRTDevice(std::string name, std::vector<IO::channel_t> channel_list)
      : RT::Device(name, channel_list)
  {
    this->bind_read_callback([&](){ return; });
    this->bind_write_callback([&](){ return; });
  }
  MOCK_METHOD(void, read, (), ());
  MOCK_METHOD(void, write, (), ());
};

class MockRTThread : public RT::Thread
{
public:
  MockRTThread(std::string name, std::vector<IO::channel_t> channel_list)
      : RT::Thread(name, channel_list)
  {
    this->bind_execute_callback([&](){ return; });
  }
  MOCK_METHOD(void, execute, (), ());
  // MOCK_METHOD(void, input, (const std::vector<double>&), (override));
  // MOCK_METHOD(const std::vector<double>&, output, (), (override));
};

class RTConnectorTest : public ::testing::Test
{
public:
  std::string defaultBlockName;
  std::string defaultInputChannelName = "CHANNEL INPUT";
  std::string defaultInputChannelDescription =
      "DEFAULT INPUT CHANNEL DESCRIPTION";
  std::string defaultOutputChannelName = "CHANNEL OUTPUT";
  std::string defaultOutputChannelDescription =
      "DEFAULT OUTPUT CHANNEL DESCRIPTION";
  std::vector<IO::channel_t> defaultChannelList;

protected:
  RTConnectorTest()
  {
    // Generates a default channels
    defaultBlockName = "DEFAULT:BLOCK:NAME";
    IO::channel_t defaultInputChannel = {};
    defaultInputChannel.name = defaultInputChannelName;
    defaultInputChannel.description = defaultInputChannelDescription;
    defaultInputChannel.flags = IO::INPUT;
    defaultInputChannel.data_size = 1;
    IO::channel_t defaultOutputChannel = {};
    defaultOutputChannel.name = defaultInputChannelName;
    defaultOutputChannel.description = defaultInputChannelDescription;
    defaultOutputChannel.flags = IO::OUTPUT;
    defaultOutputChannel.data_size = 1;
    defaultChannelList.push_back(defaultInputChannel);
    defaultChannelList.push_back(defaultOutputChannel);
  }
  RT::Connector connector;
};

#endif
