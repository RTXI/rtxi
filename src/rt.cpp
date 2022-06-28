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
#include "fifo.hpp"
#include "event.hpp"
#include "rt.hpp"

RT::System::System(Event::Manager* em, IO::Connector* ioc) 
  : event_manager(em), io_connector(ioc)
{
  if(RT::OS::getFifo(this->eventFifo, 255*sizeof(Event::Object*)) != 0){
    ERROR_MSG("RT::System::System : failed to create Fifo");
    return;
  }
  this->task = std::make_unique<RT::OS::Task>();
  if (RT::OS::createTask<RT::System*>(this->task.get(), &RT::System::execute, this) != 0) {
    ERROR_MSG("RT::System::System : failed to create realtime thread\n");
    return;
  }
  
  this->event_manager->registerHandler(this);
}

RT::System::~System()
{
  //this->task->task_finished = true;
  RT::OS::deleteTask(this->task.get());
  this->event_manager->unregisterHandler(this);
}

int64_t RT::System::getPeriod()
{
  return this->task->period;
}

void RT::System::postTelemitry(RT::Telemitry::Response telemitry)
{
  this->eventFifo->writeRT(&telemitry, sizeof(RT::Telemitry::Response));
}

RT::Telemitry::Response RT::System::getTelemitry()
{
  RT::Telemitry::Response telemitry = RT::Telemitry::NO_TELEMITRY;
  this->eventFifo->read(&telemitry, sizeof(RT::Telemitry::Response));
  return telemitry;
}

void RT::System::executeCMD(RT::System::periodUpdate* cmd)
{
  this->task->period = *(cmd->period);
  auto telem = RT::Telemitry::RT_PERIOD_UPDATE;
  this->postTelemitry(telem);
  cmd->done();
}

void RT::System::executeCMD(RT::System::deviceListUpdate* cmd)
{
  this->devices = cmd->device_list;
  auto telem = RT::Telemitry::RT_DEVICE_LIST_UPDATE;
  this->postTelemitry(telem);
  cmd->done();
}

void RT::System::executeCMD(RT::System::threadListUpdate* cmd)
{
  this->threads = cmd->thread_list;
  auto telem = RT::Telemitry::RT_THREAD_LIST_UPDATE;
  this->postTelemitry(telem);
  cmd->done();
}

void RT::System::executeCMD(shutdownCMD* cmd)
{
  this->task->task_finished = true;
  auto telem = RT::Telemitry::RT_SHUTDOWN;
  this->postTelemitry(telem);
  cmd->done();
}

void RT::System::receiveEvent(Event::Object* event)
{
  this->handleEvent(event);
}

void RT::System::handleEvent(RT::changePeriodEvent* event)
{
  RT::System::periodUpdate cmd(event->period);
  RT::System::periodUpdate* cmd_ptr = &cmd;
  this->eventFifo->write(&cmd_ptr, sizeof(RT::System::periodUpdate*));
  cmd.wait();
  event->done();
  cmd_ptr = nullptr;
}

void RT::System::handleEvent(RT::NOOPEvent* event)
{
  RT::System::NOOPCMD cmd;
  RT::System::NOOPCMD* cmd_ptr = &cmd;
  this->eventFifo->write(&cmd_ptr, sizeof(RT::System::NOOPCMD*));
  cmd.wait();
  event->done();
  cmd_ptr = nullptr;
}

void RT::System::handleEvent(RT::insertDeviceEvent* event)
{
  if (event->device == nullptr) {
    ERROR_MSG("RT::System::insertDevice : invalid device pointer\n");
    return;
  }
  this->io_connector->insertBlock(event->device);
  std::vector<IO::Block*> device_list = this->io_connector->getDevices();
  RT::System::deviceListUpdate cmd(device_list);
  RT::System::deviceListUpdate* cmd_ptr = &cmd;
  this->eventFifo->write(&cmd_ptr, sizeof(RT::System::deviceListUpdate*));
  cmd.wait();
  event->done();
  cmd_ptr = nullptr;
}

void RT::System::handleEvent(RT::removeDeviceEvent* event)
{
  if (event->device == nullptr) {
    ERROR_MSG("RT::System::removeDevice : invalid device pointer\n");
    return;
  }
  // We have to make sure to deactivate device before removing
  event->device->setActive(false);
  this->io_connector->removeBlock(event->device);
  std::vector<IO::Block*> device_list = this->io_connector->getDevices();
  RT::System::deviceListUpdate cmd(device_list);
  RT::System::deviceListUpdate* cmd_ptr = &cmd;
  this->eventFifo->write(&cmd_ptr, sizeof(RT::System::deviceListUpdate*));
  cmd.wait();
  event->done();
  cmd_ptr = nullptr;
}

void RT::System::handleEvent(RT::insertThreadEvent* event)
{
  if (event->thread == nullptr) {
    ERROR_MSG("RT::System::removeDevice : invalid device pointer\n");
    return;
  }
  this->io_connector->insertBlock(event->thread);
  std::vector<IO::Block*> thread_list = this->io_connector->getThreads();
  RT::System::threadListUpdate cmd(thread_list);
  RT::System::threadListUpdate* cmd_ptr = &cmd;
  this->eventFifo->write(&cmd_ptr, sizeof(RT::System::threadListUpdate*));
  cmd.wait();
  event->done();
  cmd_ptr = nullptr;
}

void RT::System::handleEvent(RT::removeThreadEvent* event)
{
  if (event->thread == nullptr) {
    ERROR_MSG("RT::System::removeDevice : invalid device pointer\n");
    return;
  }
  // We have to make sure to deactivate thread before removing
  event->thread->setActive(false);
  this->io_connector->removeBlock(event->thread);
  std::vector<IO::Block*> thread_list = this->io_connector->getThreads();
  RT::System::threadListUpdate cmd(thread_list);
  RT::System::threadListUpdate* cmd_ptr = &cmd;
  this->eventFifo->write(&cmd_ptr, sizeof(RT::System::threadListUpdate*));
  cmd.wait();
  event->done();
  cmd_ptr = nullptr;
}

void RT::System::execute(RT::System* system)
{
  Event::Object* cmd = nullptr;
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

    if (system->eventFifo->readRT(&cmd, sizeof(Event::Object*)) != -1) {
      do {
        system->executeCMD(cmd);
      } while (system->eventFifo->readRT(&cmd, sizeof(Event::Object*)) != -1);

      cmd = nullptr;
      devicesBegin = system->devices.begin();
      threadListBegin = system->threads.begin();
    }
  }
}
