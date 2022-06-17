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

#include "debug.hpp"
#include "rtos.hpp"
#include "event.hpp"
#include "rt.hpp"

RT::System::System(Event::Manager* manager) : eventManager(manager)
{
  if (RT::OS::createTask<RT::System*>(this->task.get(), &RT::System::execute, this) != 0) {
    ERROR_MSG("RT::System::System : failed to create realtime thread\n");
    return;
  }
  this->eventManager->registerHandler(this);
}

RT::System::~System()
{
  RT::OS::deleteTask(task.get());
  this->eventManager->unregisterHandler(this);
}

int64_t RT::System::getPeriod()
{
  return this->task->period;
}

int RT::System::setPeriod(int64_t period)
{
  int result = 0;
  Event::Object event(Event::RT_PERIOD_EVENT);
  event.setParam("period", &period);
  this->eventManager->postEvent(&event);
  event.wait();
  if(this->task->period != period){ result = -1; }
  return result;
}

void RT::System::postTelemitry(Event::Type* telemitry)
{
  this->eventFifo->writeRT(telemitry, sizeof(Event::Type*));
}

void RT::System::insertDevice(RT::Device* device)
{
  if (device == nullptr) {
    ERROR_MSG("RT::System::insertDevice : invalid device pointer\n");
    return;
  }
  Event::Object event(Event::RT_DEVICE_INSERT_EVENT);
  event.setParam("device", device);
  this->eventManager->postEvent(&event);
  event.wait();
}

void RT::System::removeDevice(RT::Device* device)
{
  if (device == nullptr) {
    ERROR_MSG("RT::System::removeDevice : invalid device pointer\n");
    return;
  }

  Event::Object event(Event::RT_DEVICE_REMOVE_EVENT);
  event.setParam("device", device);
  eventManager->postEvent(&event);
  event.wait();
}

void RT::System::insertThread(RT::Thread* thread)
{
  if (thread == nullptr) {
    ERROR_MSG("RT::System::insertThread : invalid thread pointer\n");
    return;
  }

  Event::Object event(Event::RT_THREAD_INSERT_EVENT);
  event.setParam("thread", thread);
  this->eventManager->postEvent(&event);
  event.wait();
}

void RT::System::removeThread(RT::Thread* thread)
{
  if (thread == nullptr) {
    ERROR_MSG("RT::System::removeThread : invalid thread pointer\n");
    return;
  }

  Event::Object event(Event::RT_THREAD_REMOVE_EVENT);
  event.setParam("thread", thread);
  this->eventManager->postEvent(&event);
  event.wait();
}

void RT::System::execute(RT::System* system)
{
  RT::CMD* cmd = nullptr;
  std::vector<Device*>::iterator iDevice;
  std::vector<Thread*>::iterator iThread;
  auto devicesBegin = system->devices.begin();
  auto devicesEnd = system->devices.end();
  auto threadListBegin = system->threads.begin();
  auto threadListEnd = system->threads.end();

  if (RT::OS::setPeriod(system->task.get(), RT::OS::DEFAULT_PERIOD) != 0) {
    ERROR_MSG(
        "RT::System::execute : failed to set the initial period of the "
        "realtime thread\n");
    return;
  }

  while (!system->task->task_finished) {
    RT::OS::sleepTimestep(system->task.get());

    for (iDevice = devicesBegin; iDevice != devicesEnd; ++iDevice){
      if ((*iDevice)->getActive()){
        (*iDevice)->read();
      }
    }

    for (iThread = threadListBegin; iThread != threadListEnd; ++iThread){
      if ((*iThread)->getActive()){
        (*iThread)->execute();
      }
    }

    for (iDevice = devicesBegin; iDevice != devicesEnd; ++iDevice){
      if ((*iDevice)->getActive()){
        (*iDevice)->write();
      }
    }

    if (system->eventFifo->readRT(&cmd, sizeof(RT::CMD*)) != 0) {
      do {
        system->executeCMD(cmd);
      } while (system->eventFifo->readRT(&cmd, sizeof(RT::CMD*)) != 0);

      cmd = nullptr;
      devicesBegin = system->devices.begin();
      threadListBegin = system->threads.begin();
    }
  }
}

void RT::System::executeCMD(RT::CMD* cmd)
{

}