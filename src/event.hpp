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

#ifndef EVENT_H
#define EVENT_H

#include <any>
#include <condition_variable>
#include <list>
#include <mutex>
#include <string>
#include <vector>

#include "fifo.hpp"

//! Event Oriented Classes
/*
 * Objects contained within this namespace are responsible
 *   for dispatching signals.
 */
namespace Event
{

enum Type
{
  RT_PERIOD_EVENT = 0,
  RT_PREPERIOD_EVENT,
  RT_POSTPERIOD_EVENT,
  RT_THREAD_INSERT_EVENT,
  RT_THREAD_REMOVE_EVENT,
  RT_DEVICE_INSERT_EVENT,
  RT_DEVICE_REMOVE_EVENT,
  IO_BLOCK_INSERT_EVENT,
  IO_BLOCK_REMOVE_EVENT,
  IO_LINK_INSERT_EVENT,
  IO_LINK_REMOVE_EVENT,
  WORKSPACE_PARAMETER_CHANGE_EVENT,
  PLUGIN_INSERT_EVENT,
  PLUGIN_REMOVE_EVENT,
  SETTINGS_OBJECT_INSERT_EVENT,
  SETTINGS_OBJECT_REMOVE_EVENT,
  OPEN_FILE_EVENT,
  START_RECORDING_EVENT,
  STOP_RECORDING_EVENT,
  ASYNC_DATA_EVENT,
  THRESHOLD_CROSSING_EVENT,
  START_GENICAM_RECORDING_EVENT,
  PAUSE_GENICAM_RECORDING_EVENT,
  STOP_GENICAM_RECORDING_EVENT,
  GENICAM_SNAPSHOT_EVENT,
  NOOP
};

std::string type_to_string(Type event_type);

class Object
{
public:
  explicit Object(Event::Type et);
  Object(const Object& obj) = delete; // copy constructor
  Object& operator=(const Object& obj) = delete; //copy assignment operator
  Object(Object &&) = delete; // move constructor
  Object& operator=(Object &&) = delete; // move assignment operator
  ~Object() = default;

  /*!
   * Obtains the name of the event object that was emitted.
   *
   * \return A string containing the name of event
   */
  std::string getName();

  Event::Type getType();

  /*!
   * Retrieves the paramaters values attached to the event
   *
   * \param Name The parameter name for which to retrieve the value of event
   * \return The value connected with the input key
   */
  std::any getParam(const std::string& param_name);

  /*!
   * Stores a key and value inside event object
   *
   * \param Key The name of the parameter to store inside event object
   * \param Value The value to store
   */
  void setParam(const std::string& param_name, const std::any& param_value);

  void wait();
  void done();
  bool isdone() const;

private:
  struct param
  {
    std::string name;
    std::any value;
  };

  std::vector<param> params;
  std::mutex processing_done_mut;
  std::condition_variable processing_done_cond;
  const Type event_type;
  bool processed;
};  // class Object

/*!
 * Entity that is signaled when an event is posted. This is an interface
 * that allows rtxi components and plugins to define how they receive those
 * events.
 *
 * \sa Event::Manager::postEvent()
 */
class Handler
{
public:
  /*!
   * Function that is called in non-realtime everytime an non-realtime
   *  event is posted.
   *
   * \param event The the event being posted.
   *
   * \sa Event::Object
   * \sa Event::Manager::postEvent()
   */
  virtual void receiveEvent(Object* event)=0;
};  // class Handler

/*
 * Managaes the collection of all objects waiting to
 *   receive signals from events.
 */
class Manager
{
public:
  Manager() = default;
  Manager(const Manager& manager) = delete; // copy constructor
  Manager& operator=(const Manager& manager) = delete; //copy assignment operator
  Manager(Manager &&) = default; // move constructor
  Manager& operator=(Manager &&) = default; // move assignment operator
  ~Manager();

  /*!
   * Function for posting an event to be signaled. This function
   * should only be called from non-realtime, and blocks until
   * it is been dispatched to all handlers.
   *
   * \param event The event to be posted.
   *
   * \sa Event::Handler
   * \sa Event::Object
   */
  void postEvent(Object* event);

  void registerHandler(Handler* handler);
  void unregisterHandler(Handler* handler);

private:
  std::list<Handler*> handlerList;
};  // class Manager

}  // namespace Event

#endif  // EVENT_H
