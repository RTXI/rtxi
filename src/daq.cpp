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

#include <algorithm>

#include "daq.hpp"

DAQ::Device* DAQ::Manager::loadDevice(const std::string& driver,
                                      const std::list<std::string>& params)
{
  if (driverMap.find(driver) == driverMap.end()) {
    ERROR_MSG("DAQ::Manager::loadDevice : Driver %s does not exist\n",
              driver.c_str());
    return nullptr;
  }

  DAQ::Device* device = driverMap[name]->createDevice(params);
  return device;
}

void DAQ::Manager::insertDevice(DAQ::Device* device)
{
  if (device == nullptr) {
    ERROR_MSG("DAQ::Manager::insertDevice : Invalid device\n");
    return;
  }

  if (std::find(devices.begin(), devices.end(), device) != devices.end()) {
    ERROR_MSG("DAQ::Device::insertDevice : Device already present\n");
    return;
  }

  devices.push_back(device);
}

void DAQ::Manager::removeDevice(DAQ::Device* device)
{
  if (device == nullptr) {
    ERROR_MSG("DAQ::Manager::removeDevice : Invalid device\n");
    return;
  }

  devices.remove(device);
}

void DAQ::Manager::registerDriver(Driver* driver, const std::string& name)
{
  if (driver == nullptr) {
    ERROR_MSG("DAQ::Manager::registerDriver : Invalid driver\n");
    return;
  }

  if (driverMap.find(name) != driverMap.end()) {
    ERROR_MSG("DAQ::Manager::registerDriver : Driver already registered\n");
    return;
  }
  driverMap[name] = driver;
}

void DAQ::Manager::unregisterDriver(const std::string& name)
{

  if (driverMap.find(name) == driverMap.end()) {
    ERROR_MSG("DAQ::Manager::unregisterDriver : Driver not registered\n");
    return;
  }
  driverMap.erase(name);
}

