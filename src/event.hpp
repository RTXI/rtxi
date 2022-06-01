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

#include <string>
#include <any>
#include <list>

//! Event Oriented Classes
/*
 * Objects contained within this namespace are responsible
 *   for dispatching signals.
 */
namespace Event
{

enum Type{
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
};

std::string type_to_string(Type event_type);

class Object
{
public:
  Object(Event::Type event_type);
  ~Object();

  /*!
   * Obtains the name of the event object that was emitted.
   *
   * \return A string containing the name of event
   */
  std::string getName();

  /*!
   * Retrieves the paramaters values attached to the event
   *
   * \param Name The parameter name for which to retrieve the value of event
   * \return The value connected with the input key
   */
  std::any getParam(std::string param_name) const;

  /*!
   * Stores a key and value inside event object
   *
   * \param Key The name of the parameter to store inside event object
   * \param Value The value to store
   */
  void setParam(std::string param_name, std::any param_value);

  const Type event_type;

  /*!
   * The agreed maximum number of parameters event objects are allowed to have
   */
  const static size_t MAX_PARAMS = 8;

private:
  size_t nparams;

  struct param
  {
    std::string name;
    std::any value;
  } params[MAX_PARAMS];

};  // class Object

/*!
 * Object that is signaled when an event is posted.
 *
 * \sa Event::Manager::postEvent()
 */
class Handler
{
public:
  Handler();
  virtual ~Handler();

  /*!
   * Function that is called in non-realtime everytime an non-realtime
   *  event is posted.
   *
   * \param event The the event being posted.
   *
   * \sa Event::Object
   * \sa Event::Manager::postEvent()
   */
  virtual void receiveEvent(const Object* event);

};  // class Handler

/*!
 * Object that is signaled when a realtime event is posted.
 *
 * \sa Event::Manager::postEventRT()
 */
class RTHandler
{
public:
  RTHandler();
  virtual ~RTHandler();

  /*!
   * Function that is called in realtime everytime a realtime
   *  event is posted.
   *
   * \param name The the event being posted.
   *
   * \sa Event::Object
   * \sa Event::Manager::postEventRT()
   */
  virtual void receiveEventRT(const Object* event);

};  // class RTHandler

/*
 * Managaes the collection of all objects waiting to
 *   receive signals from events.
 */
class Manager
{
public:
  /*!
   * Manager is a Singleton, which means that there can only be
   * one instance. This function returns a pointer to that
   * single instance.
   *
   * \return The instance of Manager.
   */
  static Manager* getInstance();

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
  void postEvent(const Object* event);

  /*!
   * Function for posting an event to be signaled. This function
   * should only be called from realtime, and blocks until
   * it is been dispatched to all handlers.
   *
   * \param event The event to be posted.
   *
   * \sa Event::RTHandler
   * \sa Event::Object
   */
  void postEventRT(const Object* event);


  void registerHandler(Handler* handler);
  void unregisterHandler(Handler* handler);

  void registerRTHandler(RTHandler* handler);
  void unregisterRTHandler(RTHandler* handler);

private:
  Manager();
  ~Manager();
  Manager(const Manager&) {};
  Manager& operator=(const Manager&)
  {
    return *getInstance();
  };

  static Manager* instance;

  std::list<Handler*> handlerList;
  std::list<RTHandler*> rthandlerList;

};  // class Manager

};  // namespace Event

#endif  // EVENT_H
