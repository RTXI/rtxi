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

#include <vector>

#include "rtos.hpp"
#include "fifo.hpp"
#include "event.hpp"
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
class Device : public IO::Block
{
public:
  Device(std::string n, const std::vector<IO::channel_t>& c)
      : IO::Block(std::move(n), c)
  {
  }
  virtual ~Device()=default;

  /**********************************************************
   * read & write must not be pure virtual because they can *
   *    be called during construction and destruction.      *
   **********************************************************/

  /*! \fn virtual void read(void)
   * Function called by the realtime task at the beginning of each period.
   *
   * \sa RT::System
   */
  virtual void read()=0;

  /*! \fn virtual void write(void)
   * Function called by the realtime task at the end of each period.
   *
   * \sa RT::System
   */
  virtual void write()=0;

  virtual bool getActive()=0;
  virtual void setActive(bool)=0;
};  // class Device

/*!
 * Base class for objects that are to interface with System.
 *
 * \sa RT::System
 */
class Thread : public IO::Block
{
public:
  Thread(std::string n, const std::vector<IO::channel_t>& c)
      : IO::Block(std::move(n), c)
  {
  }
  virtual ~Thread()=default;

  /*! \fn virtual void execute(void)
   * Function called periodically by the realtime task.
   *
   * \sa RT::System
   */
  virtual void execute()=0;

  virtual bool getActive()=0;
  virtual void setActive(bool)=0;

  virtual void input(const std::vector<double>& data)=0;
  virtual const std::vector<double>& output()=0;
};  // class Thread

class CMD : public Event::Object {};

/*!
 * Manages the RTOS as well as all objects that require
 *   realtime execution.
 */
class System : public Event::Handler
{
public:
  explicit System(Event::Manager* manager);
  ~System();

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
  int setPeriod(int64_t period);

  void insertDevice(Device* device);
  void removeDevice(Device* device);

  void insertThread(Thread* thread);
  void removeThread(Thread* thread);

  static void execute(RT::System* system);
  void executeCMD(RT::CMD* cmd);

private:
  /*!
   * Post a telemitry value to be received by the non-realtime
   * system. Used as an event handling result response.
   *
   * \param telemitry The event type pointer that was processed.
   *
   * \sa RT:Event
   */
  void postTelemitry(Event::Type* telemitry);

  Event::Manager* eventManager;

  std::unique_ptr<RT::OS::Task> task;

  std::vector<RT::Device*> devices;
  std::vector<RT::Thread*> threads;

  std::unique_ptr<RT::OS::Fifo> eventFifo;
};  // class System
}  // namespace RT
#endif  // RT_H
