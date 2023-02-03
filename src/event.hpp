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
#include <queue>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <vector>
#include <atomic>
#include <thread>

#include "fifo.hpp"

/*!
 * Event Oriented Classes
 *
 * Objects contained within this namespace are responsible
 *   for dispatching signals. 
 */
namespace Event
{

enum Type
{
  RT_PERIOD_EVENT = 0,
  RT_BLOCK_PAUSE_EVENT,
  RT_BLOCK_UNPAUSE_EVENT,
  RT_PREPERIOD_EVENT,
  RT_POSTPERIOD_EVENT,
  RT_GET_PERIOD_EVENT,
  RT_THREAD_INSERT_EVENT,
  RT_THREAD_REMOVE_EVENT,
  RT_DEVICE_INSERT_EVENT,
  RT_DEVICE_REMOVE_EVENT,
  RT_MODULE_PARAMETER_CHANGE_EVENT,
  RT_MODULE_STATE_CHANGE_EVENT,
  RT_SHUTDOWN_EVENT,
  IO_LINK_INSERT_EVENT,
  IO_LINK_REMOVE_EVENT,
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
  GENERIC_MODULE_EVENT,
  NOOP
};

/*!
 * converts the event type to a human readable name of event
 *
 * \param event_type type of event that was emitted
 * 
 * \returns A string representation of the event type
 */
std::string type_to_string(Type event_type);

// TODO: create a standardize way of generating events and their params

/*!
 * Token used to signal event
 * 
 * This object is able to hold metadata information about the event for
 * handlers to use. In order to properly use the token, the caller must 
 * wait for handlers to mark the event processed with wait()
 */
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

  /*!
   * Returns the type of event that was emitted.
   *
   * \return The type of event
   */
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

  /*!
   * Forces caller to wait for the event to be processed. This is needed for
   * events that carry metadata information for the handlers. This function
   * will block until the event is handled (marked done by handler)
   * 
   * \sa Event::Object::done()
   */
  void wait();

  /*!
   * Marks the event object as processed and successfully completed
   */
  void done();

  /*!
   * Marks the event object as processed but not successfully completed
   */
  void notdone();

  /*!
   * Checks whether the event has been proccessed. May return true if 
   * event was processed by apropriate handler but failed completion
   * 
   * \returns True if handled. False otherwise
   */
  bool handled() const;

  /*!
   * Checks whether the event object has been processed already. Events 
   * that have true value for processed are also handled. 
   * \returns true if processed. False otherwise
   */
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
  bool success;
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
  Manager();
  Manager(const Manager& manager) = delete; // copy constructor
  Manager& operator=(const Manager& manager) = delete; //copy assignment operator
  Manager(Manager &&) = delete; // move constructor
  Manager& operator=(Manager &&) = delete; // move assignment operator
  ~Manager();

  /*!
   * Function for posting an event to be signaled. This function
   * should only be called from non-realtime.
   *
   * \param event The event to be posted.
   *
   * \sa Event::Handler
   * \sa Event::Object
   */
  void postEvent(Object* event);

  /*!
   * Function for posting multiple events at the same time. The order
   * at which these events are posted are preserved. This is more 
   * efficient for situations where multiple events can be generated
   * from a single action(loading multiple plugins, loading settings
   * file that changes many parameters, unregister module, etc.)
   * 
   * \param events A vector of event pointers that will be published
   */
  void postEvent(const std::vector<Object*>& events);

  /*!
   * The main processing thread driver. It starts the event processing
   * loop responsible for routing events to handlers. Will mark the
   * event as done automatically regardless if it is handled or
   * not to prevent caller from waiting indefinitely.
   * 
   * \sa  Event::Object
   */
  void processEvents();

  /*!
   * Registers handler in the registry
   *
   * \param handler pointer of handler to add to registy
   */
  void registerHandler(Handler* handler);

  /*!
   * Removes handler from registry
   *
   * \param handler pointer of handler to remove from registry 
   */
  void unregisterHandler(Handler* handler);

  bool isRegistered(Handler* handler);

private:
  std::list<Handler*> handlerList;
  std::queue<Event::Object*> event_q;
  std::mutex event_mut; // Mutex for posting events
  std::condition_variable available_event_cond;
  std::atomic<bool> running = true;
  std::thread event_thread;

  // Shared mutex allows for multiple reader single writer scenarios.
  std::shared_mutex handlerlist_mut; // Mutex for modifying event handler queue
};  // class Manager

}  // namespace Event

#endif  // EVENT_H
