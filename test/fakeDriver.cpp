/*
Copyright (C) 2015 Georgia Institute of Technology

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

Created by Ivan F. Valerio <valerioif@gmail.com>

The face interface driver

This is used for testing purposes in machines that do not contain
supported hardware. It is meant to emulate a DAQ interface. No need
to use on actual installation.
*/

#include "daq.hpp"

enum physical_channel_t : uint8_t
{
  AI = 0,
  AO,
  DI,
  DO
};

class Device final : public DAQ::Device
{
public:
  Device(const Device&) = default;
  Device(Device&&) = delete;
  Device& operator=(const Device&) = delete;
  Device& operator=(Device&&) = delete;
  Device(const std::string& dev_name,
         const std::vector<IO::channel_t>& channels,
         std::string internal_name);
  ~Device() final;

  size_t getChannelCount(DAQ::ChannelType::type_t type) const final;
  bool getChannelActive(DAQ::ChannelType::type_t type,
                        DAQ::index_t index) const final;
  int setChannelActive(DAQ::ChannelType::type_t type,
                       DAQ::index_t index,
                       bool state) final;
  size_t getAnalogRangeCount(DAQ::index_t index) const final;
  size_t getAnalogReferenceCount(DAQ::index_t index) const final;
  size_t getAnalogUnitsCount(DAQ::index_t index) const final;
  size_t getAnalogDownsample(DAQ::ChannelType::type_t type,
                             DAQ::index_t index) const final;
  std::string getAnalogRangeString(DAQ::ChannelType::type_t type,
                                   DAQ::index_t index,
                                   DAQ::index_t range) const final;
  std::string getAnalogReferenceString(DAQ::ChannelType::type_t type,
                                       DAQ::index_t index,
                                       DAQ::index_t reference) const final;
  std::string getAnalogUnitsString(DAQ::ChannelType::type_t type,
                                   DAQ::index_t index,
                                   DAQ::index_t units) const final;
  double getAnalogGain(DAQ::ChannelType::type_t type,
                       DAQ::index_t index) const final;
  double getAnalogZeroOffset(DAQ::ChannelType::type_t type,
                             DAQ::index_t index) const final;
  DAQ::index_t getAnalogRange(DAQ::ChannelType::type_t type,
                              DAQ::index_t index) const final;
  DAQ::index_t getAnalogReference(DAQ::ChannelType::type_t type,
                                  DAQ::index_t index) const final;
  DAQ::index_t getAnalogUnits(DAQ::ChannelType::type_t type,
                              DAQ::index_t index) const final;
  DAQ::index_t getAnalogOffsetUnits(DAQ::ChannelType::type_t type,
                                    DAQ::index_t index) const final;
  int setAnalogGain(DAQ::ChannelType::type_t type,
                    DAQ::index_t index,
                    double gain) final;
  int setAnalogRange(DAQ::ChannelType::type_t type,
                     DAQ::index_t index,
                     DAQ::index_t range) final;
  int setAnalogZeroOffset(DAQ::ChannelType::type_t type,
                          DAQ::index_t index,
                          double offset) final;
  int setAnalogReference(DAQ::ChannelType::type_t type,
                         DAQ::index_t index,
                         DAQ::index_t reference) final;
  int setAnalogUnits(DAQ::ChannelType::type_t type,
                     DAQ::index_t index,
                     DAQ::index_t units) final;
  int setAnalogOffsetUnits(DAQ::ChannelType::type_t type,
                           DAQ::index_t index,
                           DAQ::index_t units) final;
  int setAnalogDownsample(DAQ::ChannelType::type_t type,
                          DAQ::index_t index,
                          size_t downsample) final;
  int setAnalogCounter(DAQ::ChannelType::type_t type, DAQ::index_t index) final;
  int setAnalogCalibrationValue(DAQ::ChannelType::type_t type,
                                DAQ::index_t index,
                                double value) final;
  double getAnalogCalibrationValue(DAQ::ChannelType::type_t type,
                                   DAQ::index_t index) const final;
  int setAnalogCalibrationActive(DAQ::ChannelType::type_t type,
                                 DAQ::index_t index,
                                 bool state) final;
  bool getAnalogCalibrationActive(DAQ::ChannelType::type_t type,
                                  DAQ::index_t index) const final;
  bool getAnalogCalibrationState(DAQ::ChannelType::type_t type,
                                 DAQ::index_t index) const final;
  int setDigitalDirection(DAQ::index_t index, DAQ::direction_t direction) final;

  void read() final;
  void write() final;

private:
  std::array<std::vector<physical_channel_t*>, DAQ::ChannelType::UNKNOWN>
      active_channels;

  std::array<std::vector<physical_channel_t>, 4> physical_channels_registry;
  std::array<DAQ::analog_range_t, 7> default_ranges = DAQ::get_default_ranges();
  std::array<std::string, 2> default_units = DAQ::get_default_units();

  // Used a tuple here because we want buffers to be in one place, and since not
  // all IO is float, that meant we could not use a plain vector of vectors.
  // Maybe we can just split them out to their own variables of same type for
  // speed.
  std::tuple<std::vector<double>,
             std::vector<double>,
             std::vector<uint8_t>,
             std::vector<uint8_t>>
      buffer_arrays;
};

class Driver : public DAQ::Driver
{
public:
  static DAQ::Driver* getInstance();
  void loadDevices() final;
  void unloadDevices() final;
  std::vector<DAQ::Device*> getDevices() final;

private:
  Driver();
  std::vector<Device> nidaq_devices;
};

DAQ::Driver* Driver::getInstance()
{
  static Driver instance;
  return &instance;
}

extern "C"
{
DAQ::Driver* getRTXIDAQDriver()
{
  return Driver::getInstance();
}

void deleteRTXIDAQDriver() {}
}
