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
#include "dlplugin.hpp"



//void DAQ::Manager::registerDriver(const std::string& driver_location)
//{
//  if (driver_location.empty()) {
//    ERROR_MSG("DAQ::Manager::registerDriver : Invalid driver\n");
//    return;
//  }
//
//  if (m_driver_registry.find(driver_location) != m_driver_registry.end()) {
//    return;
//  }
//  m_driver_registry[driver_location] = driver;
//}
//
//void DAQ::Manager::unregisterDriver(const std::string& name)
//{
//  if (driverMap.find(name) == driverMap.end()) {
//    ERROR_MSG("DAQ::Manager::unregisterDriver : Driver not registered\n");
//    return;
//  }
//  driverMap.erase(name);
//}
//
//void DAQ::Manager::receiveEvent(Event::Object* event)
//{
//  switch (event->getType()) {
//    case Event::Type::DAQ_DEVICE_QUERY_EVENT: {
//      auto device_name = std::any_cast<std::string>(event->getParam("name"));
//      auto* dev = this->getDevice(device_name);
//      event->setParam("device", std::any(dev));
//      break;
//    }
//    default:
//      return;
//  }
//}
