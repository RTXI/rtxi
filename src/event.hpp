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

#ifndef EVENT_H
#define EVENT_H

#include <any>
#include <atomic>
#include <condition_variable>
#include <list>
#include <mutex>
#include <queue>
#include <shared_mutex>
#include <string>
#include <thread>
#include <vector>

class eventLogger;

/*!
 * Event Oriented Classes
 *
 * Objects contained within this namespace are responsible
 *   for dispatching signals.
 *
 * All RTXI plugins and managers use the event system to communicate
 * with each other. The RT::System also uses the event system to change
 * it's own state, although it internally changes the format to account
 * for transmitting information to the real-time thread.
 */
namespace Event
{

/*!
 * All possible event types are enumerated here
 */
enum Type
{
  RT_PERIOD_EVENT = 0,
  RT_PREPERIOD_EVENT,
  RT_POSTPERIOD_EVENT,
  RT_GET_PERIOD_EVENT,
  RT_THREAD_INSERT_EVENT,
  RT_THREAD_REMOVE_EVENT,
  RT_THREAD_PAUSE_EVENT,
  RT_THREAD_UNPAUSE_EVENT,
  RT_DEVICE_INSERT_EVENT,
  RT_DEVICE_REMOVE_EVENT,
  RT_DEVICE_PAUSE_EVENT,
  RT_DEVICE_UNPAUSE_EVENT,
  RT_WIDGET_PARAMETER_CHANGE_EVENT,
  RT_WIDGET_STATE_CHANGE_EVENT,
  RT_SHUTDOWN_EVENT,
  IO_LINK_INSERT_EVENT,
  IO_LINK_REMOVE_EVENT,
  IO_BLOCK_QUERY_EVENT,
  IO_BLOCK_OUTPUTS_QUERY_EVENT,
  IO_CONNECTION_QUERY_EVENT,
  IO_ALL_CONNECTIONS_QUERY_EVENT,
  PLUGIN_INSERT_EVENT,
  PLUGIN_REMOVE_EVENT,
  DAQ_DEVICE_QUERY_EVENT,
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
  GENERIC_WIDGET_EVENT,
  MANAGER_SHUTDOWN_EVENT,
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
 * wait for event manager to mark the event processed. The caller and event
 * handler must agree on parameter names for successfull communication, or
 * else an exception is thrown. These parameters are stored with string values
 * as identifiers, and the types of these parameters must be known ahead of
 * time. Event handlers can potentially give a response through the same event
 * token by storing the result under a new key. It is not the responsibility 
 * of the event object class to store failure states.
 */
class Object
{
public:
  explicit Object(Event::Type et);
  Object(const Object& obj);  // copy constructor
  Object& operator=(const Object& obj) = delete;  // copy assignment operator
  Object(Object&&) = delete;  // move constructor
  Object& operator=(Object&&) = delete;  // move assignment operator
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
   * \sa Event::Type
   */
  Event::Type getType() const;

  /*!
   * Retrieves the parameters values attached to the event
   *
   * \param Name The parameter name for which to retrieve the value of event
   * \return The value connected with the input key
   */
  std::any getParam(const std::string& param_name) const;

  /*!
   * Stores a key and value inside event object
   *
   * \param Key The name of the parameter to store inside event object
   * \param Value The value to store
   */
  void setParam(const std::string& param_name, const std::any& param_value);

  /*!
   * Forces caller to wait for the event to be processed. 
   *
   * This is needed for events that carry metadata information for the handlers. 
   * This function will block until the event is handled (marked done by handler)
   * The wait function is automatically called by Event::Manager::postEvent() and
   * any direct call to the wait function will result in blocking forever.
   *
   * \sa Event::Object::done()
   * \sa Event::Manager::postEvent()
   */
  void wait();

  /*!
   * Marks the event object as processed and successfully completed
   * 
   * NOTE: Event::Manager automatically handles calling this and should not be
   * called directly by the user. Doing so will be an error and (hopefully) RTXI 
   * crashes. If not then good luck.
   *
   * \sa Event::Manager::postEvent()
   */
  void done();

  /*!
   * Checks whether the event object has been processed already. 
   *      Events that have true value for processed are also handled.
   *
   * \returns true if processed. False otherwise
   * \sa Event::Manager::postEvent()
   */
  bool isdone() const;

  /*!
   * Checks whether the given parameter key exists inside event token
   *
   * \param param_name The name of the parameter to search for
   * \return True if it exists, false otherwise.
   */
  bool paramExists(const std::string& param_name);

private:
  struct param
  {
    std::string name;
    std::any value;
  };

  std::vector<param> params;
  std::mutex processing_done_mut;
  std::condition_variable processing_done_cond;
  Type event_type;
  bool processed=false;
};  // class Object

/*!
 * Entity that is signaled when an event is posted. 
 *
 * This is an interface that allows rtxi components and plugins to define 
 * how they receive those events. All objects that hope to interact within
 * the rtxi environment with other objects in a non-realtime context must
 * inherit this base class.
 *
 * \sa Event::Manager::postEvent()
 */
class Handler
{
public:
  Handler() = default;
  Handler(const Handler&) = default;
  Handler(Handler&&) = delete;
  Handler& operator=(const Handler&) = default;
  Handler& operator=(Handler&&) = delete;
  virtual ~Handler() = default;
  /*!
   * Function that is called in non-realtime every time an non-realtime
   *  event is posted.
   *
   * \param event The the event being posted.
   *
   * \sa Event::Object
   * \sa Event::Manager::postEvent()
   */
  virtual void receiveEvent(Object* event) = 0;
};  // class Handler

/*
 * Managaes the collection of all objects waiting to
 *   receive signals from events.
 *
 * All event handlers must register themselves with the manager in order to
 * receive future signals. 
 */
class Manager
{
public:
  Manager();
  Manager(const Manager& manager) = delete;  // copy constructor
  Manager& operator=(const Manager& manager) =
      delete;  // copy assignment operator
  Manager(Manager&&) = delete;  // move constructor
  Manager& operator=(Manager&&) = delete;  // move assignment operator
  ~Manager();

  /*!
   * Function for posting an event to be signaled. 
   *
   * The event manager will take the object and route it to all event
   * handlers registered. This is done by passing the event through a
   * thread-safe queue, which is then processed by event handler workers.
   * This is synchronous, meaning that it blocks until all event handlers
   * return. In addition, the event manager automatically marks the event
   * as done right before returning.This function should only be called 
   * from non-realtime.
   *
   * \param event The event to be posted.
   *
   * \sa Event::Handler
   * \sa Event::Object
   */
  void postEvent(Object* event);

  /*!
   * Function for posting multiple events at the same time. 
   *
   * The order at which these events are posted are preserved. This is more
   * efficient for situations where multiple events can be generated
   * from a single action(loading multiple plugins, loading settings
   * file that changes many parameters, unregister module, etc.).
   * Will block until all events are processed.
   *
   * \param events A vector of events that will be published
   */
  void postEvent(std::vector<Object>& events);

  /*!
   * Registers handler in the registry
   *
   * \param handler pointer of handler to add to registry
   */
  void registerHandler(Handler* handler);

  /*!
   * Removes handler from registry
   *
   * \param handler pointer of handler to remove from registry
   */
  void unregisterHandler(Handler* handler);

  /*!
   * Checks Whether the handler is registered
   *
   * \param handler Pointer to handler to check registration
   * \return True if handler is registered, false otherwise.
   */
  bool isRegistered(Handler* handler);

  /*!
   * Returns a pointer to the event logger
   *
   * The event manager class also logs all messages being passed around in
   * RTXI and sends them to standard output. These logs are automatically 
   * generated in the event processor thread, and the only other part of
   * the system that requires access to the logger is the telemitry
   * processor, which will also log all telemitry received by RTXI from 
   * the RT::System class in the realtime thread.
   *
   * \return Raw pointer to eventLogger class
   *
   * \sa RT::System::getTelemitry
   */
  eventLogger* getLogger() { return this->logger.get(); }

private:
  std::list<Handler*> handlerList;
  std::queue<Event::Object*> event_q;
  std::mutex event_mut;  // Mutex for posting events
  std::condition_variable available_event_cond;
  std::atomic<bool> running = true;
  std::thread event_thread;
  std::vector<std::thread> thread_pool;

  // Shared mutex allows for multiple reader single writer scenarios.
  std::shared_mutex handlerlist_mut;  // Mutex for modifying event handler queue

  std::unique_ptr<eventLogger> logger;
};  // class Manager

}  // namespace Event

#endif  // EVENT_H
