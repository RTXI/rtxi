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

std::string Event::type_to_string(Event::Type event_type)
{
  std::string return_string = "";
  switch (event_type) {
    case Event::Type::RT_PERIOD_EVENT:
      return_string = "SYSTEM : period";
      break;
    case Event::Type::RT_PREPERIOD_EVENT:
      return_string = "SYSTEM : pre period";
      break;
    case Event::Type::RT_POSTPERIOD_EVENT:
      return_string = "SYSTEM : post period";
      break;
    case Event::Type::RT_THREAD_INSERT_EVENT:
      return_string = "SYSTEM : thread insert";
      break;
    case Event::Type::RT_THREAD_REMOVE_EVENT:
      return_string = "SYSTEM : thread remove";
      break;
    case Event::Type::RT_DEVICE_INSERT_EVENT:
      return_string = "SYSTEM : device insert";
      break;
    case Event::Type::RT_DEVICE_REMOVE_EVENT:
      return_string = "SYSTEM : device remove";
      break;
    case Event::Type::IO_BLOCK_INSERT_EVENT:
      return_string = "SYSTEM : block insert";
      break;
    case Event::Type::IO_BLOCK_REMOVE_EVENT:
      return_string = "SYSTEM : block remove";
      break;
    case Event::Type::IO_LINK_INSERT_EVENT:
      return_string = "SYSTEM : link insert";
      break;
    case Event::Type::IO_LINK_REMOVE_EVENT:
      return_string = "SYSTEM : link remove";
      break;
    case Event::Type::WORKSPACE_PARAMETER_CHANGE_EVENT:
      return_string = "SYSTEM : parameter change";
      break;
    case Event::Type::PLUGIN_INSERT_EVENT:
      return_string = "SYSTEM : plugin insert";
      break;
    case Event::Type::PLUGIN_REMOVE_EVENT:
      return_string = "SYSTEM : plugin remove";
      break;
    case Event::Type::SETTINGS_OBJECT_INSERT_EVENT:
      return_string = "SYSTEM : settings object insert";
      break;
    case Event::Type::SETTINGS_OBJECT_REMOVE_EVENT:
      return_string = "SYSTEM : settings object remove";
      break;
    case Event::Type::OPEN_FILE_EVENT:
      return_string = "SYSTEM : open file";
      break;
    case Event::Type::START_RECORDING_EVENT:
      return_string = "SYSTEM : start recording";
      break;
    case Event::Type::STOP_RECORDING_EVENT:
      return_string = "SYSTEM : stop recording";
      break;
    case Event::Type::ASYNC_DATA_EVENT:
      return_string = "SYSTEM : async data";
      break;
    case Event::Type::THRESHOLD_CROSSING_EVENT:
      return_string = "SYSTEM : threshold crossing event";
      break;
    case Event::Type::START_GENICAM_RECORDING_EVENT:
      return_string = "SYSTEM : start genicam recording";
      break;
    case Event::Type::PAUSE_GENICAM_RECORDING_EVENT:
      return_string = "SYSTEM : pause genicam recording";
      break;
    case Event::Type::STOP_GENICAM_RECORDING_EVENT:
      return_string = "SYSTEM : stop genicam recording";
      break;
    case Event::Type::GENICAM_SNAPSHOT_EVENT:
      return_string = "SYSTEM : genicam snap";
      break;
    case Event::Type::NOOP:
      return_string = "SYSTEM : no operation";
      break;
  }
  return return_string;
}

Event::Object::Object(Event::Type et)
    : event_type(et)
{
}

std::string Event::Object::getName()
{
  return Event::type_to_string(this->event_type);
}

std::any Event::Object::getParam(const std::string& param_name)
{
  for (auto& parameter : params) {
    if (parameter.name == param_name) {
      return parameter.value;
    }
  }
  return std::any();
}

void Event::Object::setParam(const std::string& param_name, const std::any& param_value)
{
  for (auto& parameter : params) {
    if (parameter.name == param_name) {
      parameter.value = param_value;
      return;
    }
  }

  param temp = {};
  temp.name = param_name;
  temp.value = param_value;
  params.push_back(temp);
}

void Event::Object::wait()
{
  std::unique_lock done_lock(this->processing_done_mut);
  processing_done_cond.wait(done_lock, [this]{return processed;});
}

void Event::Object::done()
{
  std::unique_lock done_lock(this->processing_done_mut);
  this->processed = true;
  this->processing_done_cond.notify_all();
}

Event::Manager::~Manager()
{
  this->handlerList.clear();
  this->rthandlerList.clear();
}

void Event::Manager::postEvent(const Event::Object* event)
{
  for (auto & handler : this->handlerList) {
    handler->receiveEvent(event);
  }
}

void Event::Manager::postEventRT(const Event::Object* event)
{
  for (auto & handler : this->rthandlerList) {
    handler->receiveEvent(event);
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

void Event::Manager::registerRTHandler(Handler* handler)
{
  rthandlerList.push_back(handler);
}

void Event::Manager::unregisterRTHandler(Handler* handler)
{
  auto location =
      std::find(rthandlerList.begin(), rthandlerList.end(), handler);
  rthandlerList.erase(location);
}
