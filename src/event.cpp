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

#include <debug.h>
#include <event.h>
#include <string.h>

const char *Event::RT_PERIOD_EVENT = "SYSTEM : period";
const char *Event::RT_PREPERIOD_EVENT = "SYSTEM : pre period";
const char *Event::RT_POSTPERIOD_EVENT = "SYSTEM : post period";
const char *Event::RT_THREAD_INSERT_EVENT = "SYSTEM : thread insert";
const char *Event::RT_THREAD_REMOVE_EVENT = "SYSTEM : thread remove";
const char *Event::RT_DEVICE_INSERT_EVENT = "SYSTEM : device insert";
const char *Event::RT_DEVICE_REMOVE_EVENT = "SYSTEM : device remove";
const char *Event::IO_BLOCK_INSERT_EVENT = "SYSTEM : block insert";
const char *Event::IO_BLOCK_REMOVE_EVENT = "SYSTEM : block remove";
const char *Event::IO_LINK_INSERT_EVENT = "SYSTEM : link insert";
const char *Event::IO_LINK_REMOVE_EVENT = "SYSTEM : link remove";
const char *Event::WORKSPACE_PARAMETER_CHANGE_EVENT = "SYSTEM : parameter change";
const char *Event::PLUGIN_INSERT_EVENT = "SYSTEM : plugin insert";
const char *Event::PLUGIN_REMOVE_EVENT = "SYSTEM : plugin remove";
const char *Event::SETTINGS_OBJECT_INSERT_EVENT = "SYSTEM : settings object insert";
const char *Event::SETTINGS_OBJECT_REMOVE_EVENT = "SYSTEM : settings object remove";
const char *Event::OPEN_FILE_EVENT = "SYSTEM : open file";
const char *Event::START_RECORDING_EVENT = "SYSTEM : start recording";
const char *Event::STOP_RECORDING_EVENT = "SYSTEM : stop recording";
const char *Event::ASYNC_DATA_EVENT = "SYSTEM : async data";
const char *Event::THRESHOLD_CROSSING_EVENT = "SYSTEM : threshold crossing event";

Event::Handler::Handler(void) {
    Event::Manager::getInstance()->registerHandler(this);
}

Event::Handler::~Handler(void) {
    Event::Manager::getInstance()->unregisterHandler(this);
}

void Event::Handler::receiveEvent(const Event::Object *) {}

Event::RTHandler::RTHandler(void) {
    Event::Manager::getInstance()->registerRTHandler(this);
}

Event::RTHandler::~RTHandler(void) {
    Event::Manager::getInstance()->unregisterRTHandler(this);
}

void Event::RTHandler::receiveEventRT(const Event::Object *) {}

Event::Object::Object(const char *nam) : name(nam), nparams(0) {
    memset(params,0,sizeof(params));
}

Event::Object::~Object(void) {}

void *Event::Object::getParam(const char *nam) const {
    for(size_t i=0;i<nparams;++i)
        if(!strcmp(params[i].name,nam))
            return params[i].value;
    return 0;
}

void Event::Object::setParam(const char *nam,void *val) {
    for(size_t i=0;i<nparams;++i)
        if(!strcmp(params[i].name,nam)) {
            params[i].value = val;
            return;
        }

    if(nparams >= MAX_PARAMS)
        return;

    params[nparams].name = nam;
    params[nparams].value = val;
    ++nparams;
}

Event::Manager::Manager(void){}

Event::Manager::~Manager(void) {}

void Event::Manager::postEvent(const Object *event) {
    Mutex::Locker lock(&mutex);

    for(std::list<Handler *>::iterator i = handlerList.begin(),end = handlerList.end();i != end;++i)
        (*i)->receiveEvent(event);
}

void Event::Manager::postEventRT(const Object *event) {
    for(RT::List<RTHandler>::iterator i = rthandlerList.begin(),end = rthandlerList.end();i != end;++i)
        i->receiveEventRT(event);
}

void Event::Manager::registerHandler(Handler *handler) {
    Mutex::Locker lock(&mutex);
    handlerList.insert(handlerList.end(),handler);
}

void Event::Manager::unregisterHandler(Handler *handler) {
    Mutex::Locker lock(&mutex);
    handlerList.remove(handler);
}

void Event::Manager::registerRTHandler(RTHandler *handler) {
    rthandlerList.insert(rthandlerList.end(),*handler);
}

void Event::Manager::unregisterRTHandler(RTHandler *handler) {
    rthandlerList.remove(*handler);
}

static Mutex mutex;
Event::Manager *Event::Manager::instance = 0;

Event::Manager *Event::Manager::getInstance(void) {
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
