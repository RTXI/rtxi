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
#include <string>

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


struct insertThreadEvent : public Event::Object
{
  insertThreadEvent(RT::Thread* thread) : thread(thread) {}
  RT::Thread* thread;
  const std::string event_name = std::string("RT_INSERT_THREAD_EVENT");
};

struct removeThreadEvent : public Event::Object
{
  removeThreadEvent(RT::Thread* thread) : thread(thread) {}
  RT::Thread* thread;
  const std::string event_name = std::string("RT_REMOVE_THREAD_EVENT");
};

struct insertDeviceEvent : public Event::Object
{
  insertDeviceEvent(RT::Device* device) : device(device) {}
  RT::Device* device;
  const std::string event_name = std::string("RT_INSERT_DEVICE_EVENT");
};

struct removeDeviceEvent : public Event::Object
{
  removeDeviceEvent(RT::Device* device) : device(device) {}
  RT::Device* device;
  const std::string event_name = std::string("RT_REMOVE_DEVICE_EVENT");
};

struct changePeriodEvent : public Event::Object
{
  changePeriodEvent(int64_t* period) : period(period) {}
  int64_t* period;
  const std::string event_name = std::string("RT_PERIOD_CHANGE_EVENT");
};

struct shutdownSystemEvent : Event::Object
{
  const std::string event_name = std::string("RT_SYSTEM_SHUTDOWN_EVENT");
};

struct NOOPEvent : Event::Object
{
  const std::string event_name = std::string("RT_NOOP_EVENT");
};

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
  System& operator=(const System& system) = delete;  // copy assignment operator
  System(System&&) = delete;  // move constructor
  System& operator=(System&&) = delete;  // move assignment operator
  ~System();

  int64_t getPeriod();
  RT::Telemitry::Response getTelemitry();

  void receiveEvent(Event::Object* event) override;

private:
  // We want our cmd class to be private. the only way to access
  // RT::System functions is through its event handler.
  struct threadListUpdate : public Event::Object
  {
    threadListUpdate(std::vector<IO::Block*> thread_list) : thread_list(thread_list) {}
    const std::string event_name = std::string("RT_CMD_UPDATE_THREAD_LIST");
    std::vector<IO::Block*> thread_list;
  };

  struct deviceListUpdate : public Event::Object
  {
    deviceListUpdate(std::vector<IO::Block*> device_list) : device_list(device_list) {}
    const std::string event_name = std::string("RT_CMD_UPDATE_DEVICE_LIST");
    std::vector<IO::Block*> device_list;
  };

  struct periodUpdate : public Event::Object
  {
    periodUpdate(int64_t* period) : period(period) {}
    const std::string event_name = std::string("RT_CMD_UPDATE_PERIOD");
    int64_t* period;
  };

  struct shutdownCMD : public Event::Object
  {
    const std::string event_name = std::string("RT_CMD_SHUTDOWN");
  };

  struct NOOPCMD : public Event::Object
  {
    const std::string event_name = std::string("RT_CMD_NOOP");
  };

  virtual void handleEvent(insertDeviceEvent* event);
  virtual void handleEvent(removeDeviceEvent* event);
  virtual void handleEvent(insertThreadEvent* event);
  virtual void handleEvent(removeThreadEvent* event);
  virtual void handleEvent(changePeriodEvent* event);
  virtual void handleEvent(shutdownSystemEvent* event);
  virtual void handleEvent(NOOPEvent* event);
  virtual void handleEvent(Event::Object* event);

  virtual void executeCMD(shutdownCMD* cmd);
  virtual void executeCMD(deviceListUpdate* cmd);
  virtual void executeCMD(threadListUpdate* cmd);
  virtual void executeCMD(periodUpdate* cmd);
  virtual void executeCMD(NOOPCMD* cmd);
  virtual void executeCMD(Event::Object* cmd);

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
