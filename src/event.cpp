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

#include <algorithm>

#include "event.hpp"

#include "debug.hpp"

std::string type_to_string(Event::Type event_type)
{
  std::string return_string;
  switch (event_type) {
    case Event::RT_PERIOD_EVENT:
      return_string = "SYSTEM : period";
      break;
    case Event::RT_PREPERIOD_EVENT:
      return_string = "SYSTEM : pre period";
      break;
    case Event::RT_POSTPERIOD_EVENT:
      return_string = "SYSTEM : post period";
      break;
    case Event::RT_THREAD_INSERT_EVENT:
      return_string = "SYSTEM : thread insert";
      break;
    case Event::RT_THREAD_REMOVE_EVENT:
      return_string = "SYSTEM : thread remove";
      break;
    case Event::RT_DEVICE_INSERT_EVENT:
      return_string = "SYSTEM : device insert";
      break;
    case Event::RT_DEVICE_REMOVE_EVENT:
      return_string = "SYSTEM : device remove";
      break;
    case Event::IO_BLOCK_INSERT_EVENT:
      return_string = "SYSTEM : block insert";
      break;
    case Event::IO_BLOCK_REMOVE_EVENT:
      return_string = "SYSTEM : block remove";
      break;
    case Event::IO_LINK_INSERT_EVENT:
      return_string = "SYSTEM : link insert";
      break;
    case Event::IO_LINK_REMOVE_EVENT:
      return_string = "SYSTEM : link remove";
      break;
    case Event::WORKSPACE_PARAMETER_CHANGE_EVENT:
      return_string = "SYSTEvent::EM : parameter change";
      break;
    case Event::PLUGIN_INSERT_EVENT:
      return_string = "SYSTEM : plugin insert";
      break;
    case Event::PLUGIN_REMOVE_EVENT:
      return_string = "SYSTEM : plugin remove";
      break;
    case Event::SETTINGS_OBJECT_INSERT_EVENT:
      return_string = "SYSTEvent::EM : settings object insert";
      break;
    case Event::SETTINGS_OBJECT_REMOVE_EVENT:
      return_string = "SYSTEvent::EM : settings object remove";
      break;
    case Event::OPEN_FILE_EVENT:
      return_string = "SYSTEM : open file";
      break;
    case Event::START_RECORDING_EVENT:
      return_string = "SYSTEM : start recording";
      break;
    case Event::STOP_RECORDING_EVENT:
      return_string = "SYSTEM : stop recording";
      break;
    case Event::ASYNC_DATA_EVENT:
      return_string = "SYSTEM : async data";
      break;
    case Event::THRESHOLD_CROSSING_EVENT:
      return_string = "SYSTEvent::EM : threshold crossing event";
      break;
    case Event::START_GENICAM_RECORDING_EVENT:
      return_string = "SYSTEvent::EM : start genicam recording";
      break;
    case Event::PAUSE_GENICAM_RECORDING_EVENT:
      return_string = "SYSTEvent::EM : pause genicam recording";
      break;
    case Event::STOP_GENICAM_RECORDING_EVENT:
      return_string = "SYSTEvent::EM : stop genicam recording";
      break;
    case Event::GENICAM_SNAPSHOT_EVENT:
      return_string = "SYSTEM : genicam snap";
      break;
  }
  return return_string;
}

Event::Handler::Handler()
{
  // Event::Manager::getInstance()->registerHandler(this);
}

Event::Handler::~Handler()
{
  // Event::Manager::getInstance()->unregisterHandler(this);
}

void Event::Handler::receiveEvent(const Event::Object*) {}

Event::RTHandler::RTHandler()
{
  // Event::Manager::getInstance()->registerRTHandler(this);
}

Event::RTHandler::~RTHandler()
{
  // Event::Manager::getInstance()->unregisterRTHandler(this);
}

void Event::RTHandler::receiveEventRT(const Event::Object*) {}

Event::Object::Object(Event::Type event_t)
    : event_type(event_t)
    , nparams(0)
{
  // memset(params, 0, sizeof(params));
}

Event::Object::~Object() {}

std::string Event::Object::getName()
{
  return Event::type_to_string(this->event_type);
}

std::any Event::Object::getParam(std::string param_name) const
{
  for (size_t i = 0; i < nparams; ++i)
    if (params[i].name == param_name)
      return params[i].value;
  return std::any();
}

void Event::Object::setParam(std::string param_name, std::any param_value)
{
  for (size_t i = 0; i < nparams; ++i)
    if (params[i].name == param_name) {
      params[i].value = param_value;
      return;
    }

  if (nparams >= MAX_PARAMS)
    return;

  params[nparams].name = param_name;
  params[nparams].value = param_value;
  ++nparams;
}

Event::Manager::Manager() {}

Event::Manager::~Manager()
{
  this->handlerList.clear();
  this->rthandlerList.clear();
}

void Event::Manager::postEvent(const Event::Object* event)
{
  for (auto i = this->handlerList.begin(); i != this->handlerList.end(); ++i) {
    (*i)->receiveEvent(event);
  }
}

void Event::Manager::postEventRT(const Event::Object* event)
{
  for (auto i = this->rthandlerList.begin(); i != this->rthandlerList.end();
       ++i) {
    (*i)->receiveEventRT(event);
  }
}

void Event::Manager::registerHandler(Handler* handler)
{
  handlerList.push_back(handler);
}

void Event::Manager::unregisterHandler(Handler* handler)
{
  auto location = std::find(handlerList.begin(), handlerList.end(), handler);
  handlerList.erase(location);
}

void Event::Manager::registerRTHandler(RTHandler* handler)
{
  rthandlerList.push_back(handler);
}

void Event::Manager::unregisterRTHandler(RTHandler* handler)
{
  auto location =
      std::find(rthandlerList.begin(), rthandlerList.end(), handler);
  rthandlerList.erase(location);
}

Event::Manager* Event::Manager::getInstance(void)
{
  if (instance) {
    return instance;
  }

  if (!instance) {
    static Manager manager;
    instance = &manager;
  }

  return instance;
}
