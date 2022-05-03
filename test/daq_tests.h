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

#ifndef DAQ_TESTS_H
#define DAQ_TESTS_H

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <string>
#include <list>
#include <daq.h>
#include <io.h>

#include <sstream>

class MockDAQDriver : public DAQ::Driver
{
public:
    MockDAQDriver(const std::string name) : DAQ::Driver(name) { }
    ~MockDAQDriver() { }

    MOCK_METHOD(DAQ::Device *, createDevice, (const std::list<std::string>&), (override));
};

class MockDAQDevice : public DAQ::Device
{
public:
    MockDAQDevice(std::string a, IO::channel_t *b, size_t s) : DAQ::Device(a, b, s) { }
    ~MockDAQDevice() { }

    MOCK_METHOD(size_t, getChannelCount, (DAQ::type_t), (const, override));
    MOCK_METHOD(bool, getChannelActive, (DAQ::type_t,DAQ::index_t), (const, override));
    MOCK_METHOD(int, setChannelActive, (DAQ::type_t,DAQ::index_t,bool), (override));
    MOCK_METHOD(size_t, getAnalogRangeCount, (DAQ::type_t,DAQ::index_t), (const, override));
    MOCK_METHOD(size_t, getAnalogReferenceCount, (DAQ::type_t,DAQ::index_t), (const, override));
    MOCK_METHOD(size_t, getAnalogUnitsCount, (DAQ::type_t,DAQ::index_t), (const, override));
    MOCK_METHOD(size_t, getAnalogDownsample, (DAQ::type_t,DAQ::index_t), (const, override));
    MOCK_METHOD(std::string, getAnalogRangeString, (DAQ::type_t,DAQ::index_t,DAQ::index_t), (const, override));
    MOCK_METHOD(std::string, getAnalogReferenceString, (DAQ::type_t,DAQ::index_t,DAQ::index_t), (const, override));
    MOCK_METHOD(std::string, getAnalogUnitsString, (DAQ::type_t,DAQ::index_t,DAQ::index_t), (const, override));
    MOCK_METHOD(double, getAnalogGain, (DAQ::type_t,DAQ::index_t), (const, override));
    MOCK_METHOD(double, getAnalogZeroOffset, (DAQ::type_t,DAQ::index_t), (const, override));
    MOCK_METHOD(DAQ::index_t, getAnalogRange, (DAQ::type_t,DAQ::index_t), (const, override));
    MOCK_METHOD(DAQ::index_t, getAnalogReference, (DAQ::type_t,DAQ::index_t), (const, override));
    MOCK_METHOD(DAQ::index_t, getAnalogUnits, (DAQ::type_t,DAQ::index_t), (const, override));
    MOCK_METHOD(DAQ::index_t, getAnalogOffsetUnits, (DAQ::type_t,DAQ::index_t), (const, override));
    MOCK_METHOD(int, setAnalogGain, (DAQ::type_t,DAQ::index_t,double), (override));
    MOCK_METHOD(int, setAnalogRange, (DAQ::type_t,DAQ::index_t,DAQ::index_t), (override));
    MOCK_METHOD(int, setAnalogZeroOffset, (DAQ::type_t,DAQ::index_t,double), (override));
    MOCK_METHOD(int, setAnalogReference, (DAQ::type_t,DAQ::index_t,DAQ::index_t), (override));
    MOCK_METHOD(int, setAnalogUnits, (DAQ::type_t,DAQ::index_t,DAQ::index_t), (override));
    MOCK_METHOD(int, setAnalogOffsetUnits, (DAQ::type_t,DAQ::index_t,DAQ::index_t), (override));
    MOCK_METHOD(int, setAnalogDownsample, (DAQ::type_t, DAQ::index_t, size_t), (override));
    MOCK_METHOD(int, setAnalogCounter, (DAQ::type_t, DAQ::index_t), (override));
    MOCK_METHOD(int, setAnalogCalibrationValue, (DAQ::type_t,DAQ::index_t, double), (override));
    MOCK_METHOD(double, getAnalogCalibrationValue, (DAQ::type_t,DAQ::index_t), (const, override));
    MOCK_METHOD(int, setAnalogCalibrationActive, (DAQ::type_t,DAQ::index_t, bool), (override));
    MOCK_METHOD(bool, getAnalogCalibrationActive, (DAQ::type_t,DAQ::index_t), (const, override));
    MOCK_METHOD(bool, getAnalogCalibrationState, (DAQ::type_t,DAQ::index_t), (const, override));
    MOCK_METHOD(DAQ::direction_t, getDigitalDirection, (DAQ::index_t), (const, override));
    MOCK_METHOD(int, setDigitalDirection, (DAQ::index_t,DAQ::direction_t), (override));
};

class DAQManagerTest : public ::testing::Test
{
public:
    static void callback(DAQ::Device *device, void *) { device->getChannelCount(DAQ::AI); }

protected:
    DAQManagerTest();
    ~DAQManagerTest(); 

    DAQ::Manager *daq_manager;
    std::stringstream cerr_buffer;
    std::streambuf *cerr_original_buffer;
};

#endif
