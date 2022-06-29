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

#include "event.hpp"
#include "fifo.hpp"
#include "io.hpp"
#include "rtos.hpp"

//! Realtime Oriented Classes
/*!
 * Objects contained within this namespace are responsible
 *   for managing realtime execution.
 */
namespace RT
{

namespace Telemitry
{
typedef int Response;
const Response RT_PERIOD_UPDATE = 0;
const Response RT_THREAD_LIST_UPDATE = 1;
const Response RT_DEVICE_LIST_UPDATE = 2;
const Response RT_NOOP = 3;
const Response RT_SHUTDOWN = 4;
const Response RT_ERROR = -1;
const Response NO_TELEMITRY = -2;
}  // namespace Telemitry

/*!
 * Base class for devices that are to interface with System.
 *
 * \sa RT::System
 */
class Device : public IO::Block
{
public:
  Device(std::string n, const std::vector<IO::channel_t>& c)
      : IO::Block(std::move(n), c, false)
  {
  }

  /*! \fn virtual void read(void)
   * Function called by the realtime task at the beginning of each period.
   *
   * \sa RT::System
   */
  virtual void read() = 0;

  /*! \fn virtual void write(void)
   * Function called by the realtime task at the end of each period.
   *
   * \sa RT::System
   */
  virtual void write() = 0;

  virtual bool getActive() = 0;
  virtual void setActive(bool) = 0;
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
      : IO::Block(std::move(n), c, true)
  {
  }

  /*! \fn virtual void execute(void)
   * Function called periodically by the realtime task.
   *
   * \sa RT::System
   */
  virtual void execute() = 0;

  virtual bool getActive() = 0;
  virtual void setActive(bool) = 0;

  virtual void input(const std::vector<double>& data) = 0;
  virtual const std::vector<double>& output() = 0;
};  // class Thread

/*!
 * Manages the RTOS as well as all objects that require
 *   realtime execution.
 */
class System : public Event::Handler
{
public:
  explicit System(Event::Manager* em, IO::Connector* ioc);
  System(const System& system) = delete;  // copy constructor
  System& operator=(const System& system) =
      delete;  // copy assignment operator
  System(System&&) = delete;  // move constructor
  System& operator=(System&&) = delete;  // move assignment operator
  ~System();


  int64_t getPeriod();
  RT::Telemitry::Response getTelemitry();

  void receiveEvent(Event::Object* event) override;

private:
  // We want our cmd class to be private. the only way to access
  // RT::System functions is through its event handler.
  class CMD : public Event::Object
  {
  public:
    explicit CMD(Event::Type et)
        : Event::Object(et) {};
  };

  void insertDevice(Event::Object* event);
  void removeDevice(Event::Object* event);
  void insertThread(Event::Object* event);
  void removeThread(Event::Object* event);
  void setPeriod(Event::Object* event);
  void shutdown(Event::Object* event);
  void NOOP(Event::Object* event);

  void executeCMD(CMD* cmd);
  void updateDeviceList(CMD* cmd);
  void updateThreadList(CMD* cmd);
  void setPeriod(CMD* cmd);
  void shutdown(CMD* cmd);

  void postTelemitry(const RT::Telemitry::Response telemitry);

  static void execute(RT::System* system);

  // System owns the task objet and the pipe used to communicate with it
  std::unique_ptr<RT::OS::Task> task;
  std::unique_ptr<RT::OS::Fifo> eventFifo;

  // system doesn't own any of the below variables. That's why they are
  // only pointers.
  Event::Manager* event_manager;
  IO::Connector* io_connector;

  // System's real-time loop maintains copy of device and thread pointers
  std::vector<RT::Device*> devices;
  std::vector<RT::Thread*> threads;
};  // class System
}  // namespace RT
#endif  // RT_H
