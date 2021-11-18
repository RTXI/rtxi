/*
 	 The Real-Time eXperiment Interface (RTXI)
	 Copyright (C) 2011 Georgia Institute of Technology, University of Utah, Weill Cornell Medical College

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


#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <daq_tests.h>

TEST_F(DAQManagerTest, getInstance)
{
    ASSERT_EQ(daq_manager, DAQ::Manager::getInstance());
    ASSERT_EQ(daq_manager->getInstance(), DAQ::Manager::getInstance());
}

TEST_F(DAQManagerTest, loadDevice)
{
    MockDAQDriver *testDriver = new MockDAQDriver("testDeviceDriver");
    EXPECT_CALL(*testDriver, createDevice).Times(::testing::AtLeast(2));
    std::list<std::string> deviceArgs1, deviceArgs2;
    deviceArgs1.push_back("testDeviceArgs1");
    DAQ::Device *device1 = daq_manager->loadDevice("testDeviceDriver", deviceArgs1);
    deviceArgs2.push_back("testDeviceArgs2");
    DAQ::Device *device2 = daq_manager->loadDevice("testDeviceDriver", deviceArgs2);
    delete testDriver;
}

TEST_F(DAQManagerTest, foreachDevice)
{
    IO::channel_t inputchannel = {
        "INPUT CHANNEL",
        "INPUT CHANNEL DESCRIPTION",
        IO::INPUT
    };
    IO::channel_t outputchannel = {
        "OUTPUT CHANNEL",
        "OUTPUT CHANNEL DESCRIPTION",
        IO::OUTPUT
    };
    std::vector<IO::channel_t *> channelList;
    std::vector<MockDAQDevice *> deviceList;
    for(int i = 0; i < 5; ++i)
    {
        channelList.push_back(new IO::channel_t[2]);
        channelList[i][0] = inputchannel;
        channelList[i][1] = outputchannel;
        deviceList.push_back(new MockDAQDevice(std::to_string(i), channelList[i], (size_t) 2));
    }
    //daq_manager->foreachDevice(callback, 
    for(int i = 0; i < 5; ++i)
    {
        delete[] channelList[i];
        delete deviceList[i];
    }
}


