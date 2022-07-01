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

#include <chrono>
#include <thread>
#include <random>


#include <gmock/gmock.h>

#include "system_tests.hpp"

TEST_F(RTConnectorTest, connections)
{
  // Have to build example blocks to test connector
  MockRTThread thread1("THREAD1", this->defaultChannelList);
  MockRTThread thread2("THREAD2", this->defaultChannelList);

  MockRTDevice device1("DEVICE1", this->defaultChannelList);
  MockRTDevice device2("DEVICE2", this->defaultChannelList);

  // we need to register blocks first
  this->connector.insertBlock(&thread1);
  this->connector.insertBlock(&thread2);
  this->connector.insertBlock(&device1);
  this->connector.insertBlock(&device2);

  // connect and disconnect between two blocks
  EXPECT_FALSE(this->connector.connected(&thread1, 0, &thread2, 0));
  EXPECT_FALSE(this->connector.connected(&thread1, 0, &device1, 0));
  EXPECT_FALSE(this->connector.connected(&thread1, 0, &device2, 0));
  EXPECT_FALSE(this->connector.connected(&device1, 0, &device2, 0));
  int result = 0;
  result = this->connector.connect(&thread1, 0, &thread2, 0);
  ASSERT_EQ(result, 0);
  // for threads make sure it avoids cycles
  result = this->connector.connect(&thread2, 0, &thread1, 0);
  ASSERT_EQ(result, -1);
  EXPECT_TRUE(this->connector.connected(&thread1, 0, &thread2, 0));
  EXPECT_FALSE(this->connector.connected(&device1, 0, &device2, 0));
  this->connector.disconnect(&thread1, 0, &thread2, 0);
  EXPECT_FALSE(this->connector.connected(&thread1, 0, &thread2, 0));
}

TEST_F(RTConnectorTest, getOutputs)
{
  MockRTThread outputThread(this->defaultBlockName, this->defaultChannelList);
  this->connector.insertBlock(&outputThread);
  std::vector<std::unique_ptr<RT::Thread>> inputThreads;
  for(int i=0; i<100; i++){
    inputThreads.push_back(std::make_unique<MockRTThread>("randblock", this->defaultChannelList));
    this->connector.insertBlock(inputThreads.back().get());
  }
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> distribution(0,1);
  std::vector<int> randvals(100);
  for(auto& val : randvals){ val = distribution(gen); }
  for(int i=0; i<100; i++){
    if(randvals[i] == 1){
      this->connector.connect(&outputThread, 0, inputThreads[i].get(), 0);
    }
  }
  std::vector<RT::outputs_info> output_connections = this->connector.getOutputs(&outputThread);
  int num_of_connections = std::accumulate(randvals.begin(), randvals.end(), 0);
  EXPECT_EQ(output_connections[0].output_threads.size(), num_of_connections);
  for(auto con : output_connections[0].output_threads){
    ASSERT_TRUE(this->connector.connected(&outputThread, 0, con.dest, 0));
  }
}

TEST_F(RTConnectorTest, getBlocks)
{
  std::vector<std::unique_ptr<RT::Thread>> threads(50);
  std::vector<std::unique_ptr<RT::Device>> devices(50);
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> distribution(0,49);
  for(int i=0; i<50; i++){
    devices[i] = std::make_unique<MockRTDevice>("randdevice", this->defaultChannelList);
    threads[i] = std::make_unique<MockRTThread>("randthread", this->defaultChannelList);
    this->connector.insertBlock(threads[i].get());
    this->connector.insertBlock(devices[i].get());
  }
  for(int iter = 0; iter < 50; iter++){
    this->connector.connect(threads[iter].get(), 0, threads[distribution(gen)].get(), 0);
    this->connector.connect(devices[iter].get(), 0, threads[distribution(gen)].get(), 0);
    this->connector.connect(threads[iter].get(), 0, devices[distribution(gen)].get(), 0);
  }
  std::vector<RT::Thread*> received_threads = this->connector.getThreads();
  std::vector<RT::Device*> received_devices = this->connector.getDevices();
  ASSERT_EQ(received_threads.size(), 50);
  ASSERT_EQ(received_devices.size(), 50);

  // verify that thread objects are in topological order
  RT::outputs_info tempinfo{};
  for(auto thread_iter=received_threads.begin(); thread_iter != received_threads.end(); thread_iter++){
    tempinfo = this->connector.getOutputs(*thread_iter)[0];
    for(auto output_thread : tempinfo.output_threads){
      auto loc = std::find_if(received_threads.begin(), 
                              thread_iter,
                              [&output_thread](RT::Thread* current_thread){
                                return current_thread == output_thread.dest;
                              });
      ASSERT_EQ(loc, thread_iter);
    }
  }
}

TEST_F(SystemTest, checkTelemitry)
{
  Event::Object event(Event::Type::NOOP);
  this->system->receiveEvent(&event);
  event.wait();
  ASSERT_EQ(RT::Telemitry::RT_NOOP, this->system->getTelemitry());
}

TEST_F(SystemTest, shutdown)
{
  Event::Object ev(Event::Type::RT_SHUTDOWN_EVENT);
  this->system->receiveEvent(&ev);
  ev.wait();
  ASSERT_EQ(this->system->getTelemitry(), RT::Telemitry::RT_SHUTDOWN);
}

TEST_F(SystemTest, getPeriod)
{
  // Check with default period
  auto period = 1000000ll;
  ASSERT_EQ(period, system->getPeriod());
}

TEST_F(SystemTest, setPeriod)
{
  Event::Object ev(Event::Type::RT_PERIOD_EVENT);
  ev.setParam("period", RT::OS::DEFAULT_PERIOD/2);
  this->system->receiveEvent(&ev);
  ev.wait();
  EXPECT_EQ(RT::Telemitry::RT_PERIOD_UPDATE, this->system->getTelemitry());
  ASSERT_EQ(RT::OS::DEFAULT_PERIOD/2, system->getPeriod());
  ev.setParam("period", RT::OS::DEFAULT_PERIOD);
  this->system->receiveEvent(&ev);
  ev.wait();
  EXPECT_EQ(RT::Telemitry::RT_PERIOD_UPDATE, this->system->getTelemitry());
  ASSERT_EQ(RT::OS::DEFAULT_PERIOD, system->getPeriod());
}

TEST_F(SystemTest, updateDeviceList)
{
  std::string defaultInputChannelName = "CHANNEL INPUT";
  std::string defaultInputChannelDescription =
      "DEFAULT INPUT CHANNEL DESCRIPTION";
  std::string defaultOutputChannelName = "CHANNEL OUTPUT";
  std::string defaultOutputChannelDescription =
      "DEFAULT OUTPUT CHANNEL DESCRIPTION";
  std::vector<IO::channel_t> defaultChannelList;

  // Generates a default block with single input and output channel
  IO::channel_t defaultInputChannel = {};
  defaultInputChannel.name = defaultInputChannelName;
  defaultInputChannel.description = defaultInputChannelDescription;
  defaultInputChannel.flags = IO::INPUT;
  defaultInputChannel.data_size = 1;
  IO::channel_t defaultOutputChannel = {};
  defaultOutputChannel.name = defaultOutputChannelName;
  defaultOutputChannel.description = defaultOutputChannelDescription;
  defaultOutputChannel.flags = IO::OUTPUT;
  defaultOutputChannel.data_size = 1;
  defaultChannelList.push_back(defaultInputChannel);
  defaultChannelList.push_back(defaultOutputChannel);

  MockRTDevice mock_device("mockdevice", defaultChannelList);
  RT::Device* device_ptr = &mock_device;

  // insert device
  this->rt_connector->insertBlock(device_ptr);
  Event::Object insertEvent(Event::Type::RT_DEVICE_INSERT_EVENT);
  insertEvent.setParam("device",  device_ptr);
  this->system->receiveEvent(&insertEvent);
  insertEvent.wait();
  ASSERT_EQ(this->system->getTelemitry(), RT::Telemitry::RT_DEVICE_LIST_UPDATE);
  ASSERT_TRUE(this->rt_connector->isRegistered(&mock_device));

  // remove device
  Event::Object removeEvent(Event::Type::RT_DEVICE_REMOVE_EVENT);
  removeEvent.setParam("device", device_ptr);
  this->system->receiveEvent(&removeEvent);
  removeEvent.wait();
  ASSERT_EQ(this->system->getTelemitry(), RT::Telemitry::RT_DEVICE_LIST_UPDATE);
  ASSERT_FALSE(this->rt_connector->isRegistered(&mock_device));

}
