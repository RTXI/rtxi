/*
 * Copyright (C) 2005 Boston University
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License as
 *  published by the Free Software Foundation; either version 2 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <compiler.h>
#include <daq.h>
#include <algorithm>

DAQ::Device::Device(std::string name,IO::channel_t *chan,size_t size)
    : IO::Block(name,chan,size) {
    DAQ::Manager::insertDevice(this);
}

DAQ::Device::~Device(void) {
    DAQ::Manager::removeDevice(this);
}

DAQ::Driver::Driver(const std::string &n)
    : name(n) {
    DAQ::Manager::registerDriver(this,name);
}

DAQ::Driver::~Driver(void) {
    DAQ::Manager::unregisterDriver(name);
}

void DAQ::Manager::foreachDevice(void (*callback)(DAQ::Device *,void *),void *param) {
    if(unlikely(!instance))
        initialize();

    Mutex::Locker lock(&instance->mutex);
    for(std::list<Device *>::iterator i = instance->deviceList.begin();i != instance->deviceList.end();++i)
        callback(*i,param);
}

DAQ::Device *DAQ::Manager::loadDevice(const std::string &name,const std::list<std::string> &args) {
    if(unlikely(!instance))
        initialize();

    Mutex::Locker lock(&instance->mutex);

    if(instance->driverMap.find(name) == instance->driverMap.end()) {
        ERROR_MSG("DAQ::Manager::loadDevice : driver %s does not exist\n",name.c_str());
        return 0;
    }

    DAQ::Device *device = instance->driverMap[name]->createDevice(args);
    return device;
}

void DAQ::Manager::insertDevice(DAQ::Device *device) {
    if(unlikely(!instance))
        initialize();

    if(!device) {
        ERROR_MSG("DAQ::Manager::insertDevice : invalid device\n");
        return;
    }

    Mutex::Locker lock(&instance->mutex);

    if(std::find(instance->deviceList.begin(),instance->deviceList.end(),device) != instance->deviceList.end()) {
        ERROR_MSG("DAQ::Device::insertDevice : device already present\n");
        return;
    }

    instance->deviceList.push_back(device);
}

void DAQ::Manager::removeDevice(DAQ::Device *device) {
    if(unlikely(!instance))
        initialize();

    if(!device) {
        ERROR_MSG("DAQ::Manager::removeDevice : invalid device\n");
        return;
    }

    Mutex::Locker lock(&instance->mutex);
    instance->deviceList.remove(device);
}

void DAQ::Manager::registerDriver(Driver *driver,const std::string &name) {
    if(unlikely(!instance))
        initialize();

    if(!driver) {
        ERROR_MSG("DAQ::Manager::registerDriver : invalid driver\n");
        return;
    }

    Mutex::Locker lock(&instance->mutex);

    if(instance->driverMap.find(name) != instance->driverMap.end()) {
        ERROR_MSG("DAQ::Manager::registerDriver : driver already registered\n");
        return;
    }

    instance->driverMap[name] = driver;
}

void DAQ::Manager::unregisterDriver(const std::string &name) {
    if(unlikely(!instance))
        initialize();

    Mutex::Locker lock(&instance->mutex);

    if(instance->driverMap.find(name) == instance->driverMap.end()) {
        ERROR_MSG("DAQ::Manager::unregisterDriver : driver not registered\n");
        return;
    }

    instance->driverMap.erase(name);
}

static Mutex mutex;
DAQ::Manager *DAQ::Manager::instance = 0;

void DAQ::Manager::initialize(void) {
    if(instance)
        return;

    Mutex::Locker lock(&::mutex);
    if(!instance) {
        static Manager manager;
        instance = &manager;
    }
}
