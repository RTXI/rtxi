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

/*!
 * Event Oriented Classes
 *
 * Objects contained within this namespace are responsible
 *   for dispatching signals. 
 */
namespace Event
{

/*!
 * Token used to signal event
 * 
 * This object is able to hold metadata information about the event for
 * handlers to use. In order to properly use the token, the caller must 
 * wait for handlers to mark the event processed with wait()
 */
struct Object
{
public:
  explicit Object() : processed(false) {};
  Object(const Object& obj) = delete; // copy constructor
  Object& operator=(const Object& obj) = delete; //copy assignment operator
  Object(Object &&) = delete; // move constructor
  Object& operator=(Object &&) = delete; // move assignment operator
  ~Object() = default;

  /*!
   * Forces caller to wait for the event to be processed. This is needed for
   * events that carry metadata information for the handlers. This function
   * will block until the event is handled (marked done by handler)
   * 
   * \sa Event::Object::done()
   */
  void wait();

  /*!
   * Marks the event object as processed.
   */
  void done();

  /*!
   * Checks whether the event object has been processed already. This can be an
   * alternative for situations where waiting for event is not neccessary and
   * caller just wishes to check event processing completion.
   * 
   * \returns true if processed. False otherwise
   */
  bool isdone() const;
  
  const std::string event_name = "NOOP";
private:
  std::mutex processing_done_mut;
  std::condition_variable processing_done_cond;
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
  virtual void receiveEvent(Event::Object* event)=0;
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
   * should only be called from non-realtime.
   *
   * \param event The event to be posted.
   *
   * \sa Event::Handler
   * \sa Event::Object
   */
  void postEvent(Object* event);

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

private:
  std::list<Handler*> handlerList;
};  // class Manager

}  // namespace Event

#endif  // EVENT_H
