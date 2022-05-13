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

#ifndef RT_H
#define RT_H

#include <thread>

#include "fifo.hpp"
#include "mutex.hpp"
#include "rtos.hpp"

//! Realtime Oriented Classes
/*!
 * Objects contained within this namespace are responsible
 *   for managing realtime execution.
 */
namespace RT
{

/*!
 * A token passed to the realtime task through System::postEvent()
 *   for synchronization.
 *
 * \sa RT::System::postEvent()
 */
class Event
{
public:
  Event();
  virtual ~Event();

  /*!
   * Function called by the realtime task in System::postEvent()
   *
   * \sa RT::System::postEvent()
   */
  virtual int callback() = 0;
  void execute();
  void wait();

  int retval;
private:

};  // class Event

template<typename T>
class List
{
public:
  class Node;

  class iterator
  {
  public:
    iterator()
        : current(0) {};
    iterator(T* x)
        : current(static_cast<Node*>(x)) {};
    iterator(const iterator& x)
        : current(x.current) {};

    bool operator==(const iterator& x) const
    {
      return current == x.current;
    };
    bool operator!=(const iterator& x) const
    {
      return current != x.current;
    };

    T& operator*() const
    {
      return *static_cast<T*>(current);
    };
    T* operator->() const
    {
      return static_cast<T*>(current);
    };

    iterator& operator++()
    {
      current = current->next;
      return *this;
    };
    iterator operator++(int)
    {
      typename RT::List<T>::iterator tmp = *this;
      current = current->next;
      return tmp;
    };
    iterator& operator--()
    {
      current = current->prev;
      return *this;
    };
    iterator operator--(int)
    {
      typename RT::List<T>::iterator tmp = *this;
      current = current->prev;
      return tmp;
    };

  private:
    Node* current;

  };  // class iterator

  class const_iterator
  {
  public:
    const_iterator()
        : current(0) {};
    const_iterator(const T* x)
        : current(static_cast<const Node*>(x)) {};
    const_iterator(const const_iterator& x)
        : current(x.current) {};

    bool operator==(const const_iterator& x) const
    {
      return current == x.current;
    };
    bool operator!=(const const_iterator& x) const
    {
      return current != x.current;
    };

    const T& operator*() const
    {
      return *static_cast<const T*>(current);
    };
    const T* operator->() const
    {
      return static_cast<const T*>(current);
    };

    const_iterator& operator++()
    {
      current = current->next;
      return *this;
    };
    const_iterator operator++(int)
    {
      typename RT::List<T>::const_iterator tmp = *this;
      current = current->next;
      return tmp;
    };
    const_iterator& operator--()
    {
      current = current->prev;
      return *this;
    };
    const_iterator operator--(int)
    {
      typename RT::List<T>::const_iterator tmp = *this;
      current = current->prev;
      return tmp;
    };

  private:
    const Node* current;

  };  // class const_iterator

  class Node
  {
    friend class List<T>;
    friend class List<T>::iterator;
    friend class List<T>::const_iterator;

  public:
    Node()
        : next(0)
        , prev(0) {};
    virtual ~Node() {};

    bool operator==(const Node& x) const
    {
      return next == x.next && prev == x.prev;
    };

  private:
    Node *next, *prev;

  };  // class Node

  List()
      : count(0)
      , head(&tail)
      , tail() {};
  virtual ~List()
  {
#ifdef DEBUG
    if (tail.next)
      ERROR_MSG("RT::List::~List : end of list overwritten\n");
#endif
  };

  size_t size() const
  {
    return count;
  };
  bool empty() const
  {
    return count == 0;
  };

  iterator begin()
  {
    return iterator(static_cast<T*>(head));
  };
  iterator end()
  {
    return iterator(static_cast<T*>(&tail));
  };

  const_iterator begin() const
  {
    return const_iterator(static_cast<const T*>(head));
  };
  const_iterator end() const
  {
    return const_iterator(static_cast<const T*>(&tail));
  };

  void insert(iterator, T&);
  void insertRT(iterator position, T& node)
  {
    Node* object = static_cast<Node*>(&node);

    object->next = &(*position);
    object->prev = object->next->prev;
    if (object->next == head)
      head = object;
    else
      position->prev->next = object;
    position->prev = object;
    count++;
  };

  void remove(T&);
  void removeRT(T& node)
  {
    Node* object = static_cast<Node*>(&node);

    if (object == &tail)
      return;
    if (object == head)
      head = object->next;
    else if (object->prev)
      object->prev->next = object->next;
    object->next->prev = object->prev;
    count--;
  };

private:
  class InsertListNodeEvent : public RT::Event
  {
  public:
    InsertListNodeEvent(List<T>* l, iterator i, T* n)
        : list(l)
        , iter(i)
        , node(n) {};
    int callback()
    {
      list->insertRT(iter, *node);
      return 0;
    };

  private:
    List<T>* list;
    typename List<T>::iterator iter;
    T* node;

  };  // class InsertListNodeEvent;

  class RemoveListNodeEvent : public RT::Event
  {
  public:
    RemoveListNodeEvent(List<T>* l, T* n)
        : list(l)
        , node(n) {};
    int callback()
    {
      list->removeRT(*node);
      return 0;
    };

  private:
    List<T>* list;
    T* node;

  };  // class RemoveListNodeEvnet;

  size_t count;
  Node *head, tail;

};  // class List


/*!
 * Base class for devices that are to interface with System.
 *
 * \sa RT::System
 */
class Device : public List<Device>::Node
{
public:
  Device(void);
  virtual ~Device();

  /**********************************************************
   * read & write must not be pure virtual because they can *
   *    be called during construction and destruction.      *
   **********************************************************/

  /*! \fn virtual void read(void)
   * Function called by the realtime task at the beginning of each period.
   *
   * \sa RT::System
   */
  virtual void read() {};

    /*! \fn virtual void write(void)
   * Function called by the realtime task at the end of each period.
   *
   * \sa RT::System
   */
  virtual void write() {};

  inline bool getActive() const
  {
    return active;
  };
  void setActive(bool);

private:
  bool active;

};  // class Device

/*!
 * Base class for objects that are to interface with System.
 *
 * \sa RT::System
 */
class Thread : public List<Thread>::Node
{
public:
  typedef unsigned long Priority;

  static const Priority MinimumPriority = 0;
  static const Priority MaximumPriority = 100;
  static const Priority DefaultPriority = MaximumPriority / 2;

  Thread(Priority p = DefaultPriority);
  virtual ~Thread(void);

  /*!
   * Returns the priority of the thread. The higher the
   *   priority the sooner the thread is called in the
   *   timestep.
   *
   * \return The priority of the thread.
   */
  Priority getPriority(void) const
  {
    return priority;
  };

  /*! \fn virtual void execute(void)
   * Function called periodically by the realtime task.
   *
   * \sa RT::System
   */

  /**********************************************************
   * execute must not be pure virtual because it can be     *
   *   called during construction and destruction.          *
   **********************************************************/

  virtual void execute(void) {};

  inline bool getActive(void) const
  {
    return active;
  };
  void setActive(bool);

private:
  bool active;
  Priority priority;

};  // class Thread

/*!
 * Manages the RTOS as well as all objects that require
 *   realtime execution.
 */
class System
{
public:
  /*!
   * System is a Singleton, which means that there can only be one instance.
   *   This function returns a pointer to that single instance.
   *
   * \return The instance of System.
   */
  static System* getInstance();

  /*!
   * Get the current period of the System in nanoseconds.
   *
   * \return The current period
   */
  int64_t getPeriod();

  /*!
   * Set a new period for the System in nanoseconds.
   *
   * \param period The new desired period.
   * \return 0 on success, A negative value upon failure.
   */
  int setPeriod(long long period);

  /*!
   * Loop through each Device and executes a callback.
   * The callback takes two parameters, a Device pointer and param,
   *   the second parameter to foreachDevice.
   *
   * \param callback The callback function.
   * \param param A parameter to the callback function.
   * \sa RT::Device
   */
  void foreachDevice(void (*callback)(Device*, void*), void* param);
  /*!
   * Loop through each Thread and executes a callback.
   * The callback takes two parameters, a Thread pointer and param,
   *   the second parameter to foreachThread.
   *
   * \param callback The callback function
   * \param param A parameter to the callback function
   * \sa RT::Thread
   */
  void foreachThread(void (*callback)(Thread*, void*), void* param);

  /*!
   * Post an Event for execution by the realtime task, this acts as a
   *   mechanism to synchronizing with the realtime task.
   *
   * \param event The event to be posted.
   * \param blocking If true the call to postEvent is blocking.
   * \return The value returned from event->callback()
   * \sa RT:Event
   */
  int postEvent(Event* event, bool blocking = true);

private:
  /******************************************************************
   * The constructors, destructor, and assignment operator are made *
   *   private to control instantiation of the class.               *
   ******************************************************************/

  System(void);
  ~System(void);
  System(const System&)
      : eventFifo(0) {};
  System& operator=(const System&)
  {
    return *getInstance();
  };

  class SetPeriodEvent : public RT::Event
  {
  public:
    SetPeriodEvent(long long);
    ~SetPeriodEvent(void);

    int callback(void);

  private:
    long long period;

  };  // class SetPeriodEvent

  static System* instance;

  Mutex deviceMutex;
  void insertDevice(Device*);
  void removeDevice(Device*);

  Mutex threadMutex;
  void insertThread(Thread*);
  void removeThread(Thread*);

  static void* bounce(void*);
  void execute(void);

  RT::OS::Task *task;

  List<RT::Device> devices;
  List<RT::Thread> threadList;

  Fifo eventFifo;

};  // class System

template<typename T>
void List<T>::insert(iterator position, T& node)
{
  InsertListNodeEvent event(this, position, &node);
  RT::System::getInstance()->postEvent(&event);
}

template<typename T>
void List<T>::remove(T& node)
{
  RemoveListNodeEvent event(this, &node);
  RT::System::getInstance()->postEvent(&event);
}

}  // namespace RT

#endif  // RT_H
