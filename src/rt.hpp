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

#include "fifo.hpp"
#include "rtos.hpp"
#include "io.hpp"

//! Realtime Oriented Classes
/*!
 * Objects contained within this namespace are responsible
 *   for managing realtime execution.
 */
namespace RT
{

/*!
 * Base class for devices that are to interface with System.
 *
 * \sa RT::System
 */
class Device
{
public:
  Device();
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
class Thread
{
public:
  Thread();
  virtual ~Thread();

  /*! \fn virtual void execute(void)
   * Function called periodically by the realtime task.
   *
   * \sa RT::System
   */
  virtual void execute(void) {};

  inline bool getActive(void) const
  {
    return active;
  };
  void setActive(bool);

  void input(const std::vector<double>& data);
  const std::vector<double>& output();

protected:
  std::unique_ptr<IO::Block> dataBlock;
  
private:
  bool active;

};  // class Thread

// /*!
//  * Manages the RTOS as well as all objects that require
//  *   realtime execution.
//  */
// class System
// {
// public:
//   /*!
//    * System is a Singleton, which means that there can only be one instance.
//    *   This function returns a pointer to that single instance.
//    *
//    * \return The instance of System.
//    */
//   static System* getInstance();

//   /*!
//    * Get the current period of the System in nanoseconds.
//    *
//    * \return The current period
//    */
//   int64_t getPeriod();

//   /*!
//    * Set a new period for the System in nanoseconds.
//    *
//    * \param period The new desired period.
//    * \return 0 on success, A negative value upon failure.
//    */
//   int setPeriod(long long period);

//   /*!
//    * Loop through each Device and executes a callback.
//    * The callback takes two parameters, a Device pointer and param,
//    *   the second parameter to foreachDevice.
//    *
//    * \param callback The callback function.
//    * \param param A parameter to the callback function.
//    * \sa RT::Device
//    */
//   void foreachDevice(void (*callback)(Device*, void*), void* param);
//   /*!
//    * Loop through each Thread and executes a callback.
//    * The callback takes two parameters, a Thread pointer and param,
//    *   the second parameter to foreachThread.
//    *
//    * \param callback The callback function
//    * \param param A parameter to the callback function
//    * \sa RT::Thread
//    */
//   void foreachThread(void (*callback)(Thread*, void*), void* param);

//   /*!
//    * Post an Event for execution by the realtime task, this acts as a
//    *   mechanism to synchronizing with the realtime task.
//    *
//    * \param event The event to be posted.
//    * \param blocking If true the call to postEvent is blocking.
//    * \return The value returned from event->callback()
//    * \sa RT:Event
//    */
//   int postEvent(Event* event, bool blocking = true);

// private:
//   /******************************************************************
//    * The constructors, destructor, and assignment operator are made *
//    *   private to control instantiation of the class.               *
//    ******************************************************************/

//   System();
//   ~System();

//   Mutex deviceMutex;
//   void insertDevice(Device*);
//   void removeDevice(Device*);

//   Mutex threadMutex;
//   void insertThread(Thread*);
//   void removeThread(Thread*);

//   static void bounce(RT::System* param);
//   void execute(void);

//   RT::OS::Task *task;

//   List<RT::Device> devices;
//   List<RT::Thread> threadList;

//   Fifo eventFifo;

// };  // class System

}  // namespace RT

#endif  // RT_H
