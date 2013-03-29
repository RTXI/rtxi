/*
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

#include <daq.h>
#include <algorithm>

DAQ::Device::Device(std::string name,IO::channel_t *chan,size_t size)
    : IO::Block(name,chan,size) {
    DAQ::Manager::getInstance()->insertDevice(this);
}

DAQ::Device::~Device(void) {
    DAQ::Manager::getInstance()->removeDevice(this);
}

DAQ::Driver::Driver(const std::string &n)
    : name(n) {
    DAQ::Manager::getInstance()->registerDriver(this,name);
}

DAQ::Driver::~Driver(void) {
    DAQ::Manager::getInstance()->unregisterDriver(name);
}

void DAQ::Manager::foreachDevice(void (*callback)(DAQ::Device *,void *),void *param) {
    Mutex::Locker lock(&mutex);
    for(std::list<Device *>::iterator i = deviceList.begin();i != deviceList.end();++i)
        callback(*i,param);
}

DAQ::Device *DAQ::Manager::loadDevice(const std::string &name,const std::list<std::string> &args) {
    Mutex::Locker lock(&mutex);

    if(driverMap.find(name) == driverMap.end()) {
        ERROR_MSG("DAQ::Manager::loadDevice : driver %s does not exist\n",name.c_str());
        return 0;
    }

    DAQ::Device *device = driverMap[name]->createDevice(args);
    return device;
}

void DAQ::Manager::insertDevice(DAQ::Device *device) {
    if(!device) {
        ERROR_MSG("DAQ::Manager::insertDevice : invalid device\n");
        return;
    }

    Mutex::Locker lock(&mutex);

    if(std::find(deviceList.begin(),deviceList.end(),device) != deviceList.end()) {
        ERROR_MSG("DAQ::Device::insertDevice : device already present\n");
        return;
    }

    deviceList.push_back(device);
}

void DAQ::Manager::removeDevice(DAQ::Device *device) {
    if(!device) {
        ERROR_MSG("DAQ::Manager::removeDevice : invalid device\n");
        return;
    }

    Mutex::Locker lock(&mutex);
    deviceList.remove(device);
}

void DAQ::Manager::registerDriver(Driver *driver,const std::string &name) {
    if(!driver) {
        ERROR_MSG("DAQ::Manager::registerDriver : invalid driver\n");
        return;
    }

    Mutex::Locker lock(&mutex);

    if(driverMap.find(name) != driverMap.end()) {
        ERROR_MSG("DAQ::Manager::registerDriver : driver already registered\n");
        return;
    }

    driverMap[name] = driver;
}

void DAQ::Manager::unregisterDriver(const std::string &name) {
    Mutex::Locker lock(&mutex);

    if(driverMap.find(name) == driverMap.end()) {
        ERROR_MSG("DAQ::Manager::unregisterDriver : driver not registered\n");
        return;
    }

    driverMap.erase(name);
}

static Mutex mutex;
DAQ::Manager *DAQ::Manager::instance = 0;

DAQ::Manager *DAQ::Manager::getInstance(void) {
    if(instance)
        return instance;

    /*************************************************************************
     * Seems like alot of hoops to jump through, but static allocation isn't *
     *   thread-safe. So effort must be taken to ensure mutual exclusion.    *
     *************************************************************************/

    Mutex::Locker lock(&::mutex);
    if(!instance) {
        static Manager manager;
        instance = &manager;
    }

    return instance;
}
