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

#include <queue>

#include "rt.hpp"

#include "debug.hpp"
#include "event.hpp"
#include "fifo.hpp"
#include "module.hpp"
#include "rtos.hpp"

int RT::Connector::connect(RT::Thread* src,
                           size_t out,
                           RT::Thread* dest,
                           size_t in)
{
  // Let's remind our users to register their block first
  if (!(this->isRegistered(src) && this->isRegistered(dest))) {
    ERROR_MSG(
        "RT::Connector : source or destination threads are not regsitered");
    return -1;
  }
  // First we must make sure that we aren't going to create a cycle
  if (dest == src) {
    return -1;
  }  // can't be connected to itself
  auto connected_blocks = std::vector<RT::Thread*>();
  auto processing = std::queue<RT::Thread*>();
  processing.push(dest);
  auto loc = std::vector<RT::Thread*>::iterator();
  while (!processing.empty()) {
    connected_blocks.push_back(processing.front());
    loc = std::find(connected_blocks.begin(), connected_blocks.end(), src);
    if (loc != connected_blocks.end()) {
      return -1;
    }
    for (const auto& connections : this->thread_registry[processing.front()]) {
      for (auto thread_connection : connections.output_threads) {
        processing.push(thread_connection.dest);
      }
    }
    processing.pop();
  }

  if (!(this->connected(src, out, dest, in))) {
    thread_connection_t tempcon = {dest, in};
    this->thread_registry[src][out].output_threads.push_back(tempcon);
  }
  return 0;
}

int RT::Connector::connect(RT::Thread* src,
                           size_t out,
                           RT::Device* dest,
                           size_t in)
{
  // Let's remind our users to register their block first
  if (!(this->isRegistered(src) && this->isRegistered(dest))) {
    ERROR_MSG(
        "RT::Connector : source or destination threads are not regsitered");
    return -1;
  }

  if (!(this->connected(src, out, dest, in))) {
    device_connection_t tempcon = {dest, in};
    this->thread_registry[src][out].output_devices.push_back(tempcon);
  }
  return 0;
}

int RT::Connector::connect(RT::Device* src,
                           size_t out,
                           RT::Thread* dest,
                           size_t in)
{
  // Let's remind our users to register their block first
  if (!(this->isRegistered(src) && this->isRegistered(dest))) {
    ERROR_MSG(
        "RT::Connector : source or destination threads are not regsitered");
    return -1;
  }

  if (!(this->connected(src, out, dest, in))) {
    thread_connection_t tempcon = {dest, in};
    this->device_registry[src][out].output_threads.push_back(tempcon);
  }
  return 0;
}

int RT::Connector::connect(RT::Device* src,
                           size_t out,
                           RT::Device* dest,
                           size_t in)
{
  // Let's remind our users to register their block first
  if (!(this->isRegistered(src) && this->isRegistered(dest))) {
    ERROR_MSG(
        "RT::Connector : source or destination threads are not regsitered");
    return -1;
  }

  if (!(this->connected(src, out, dest, in))) {
    device_connection_t tempcon = {dest, in};
    this->device_registry[src][out].output_devices.push_back(tempcon);
  }
  return 0;
}

bool RT::Connector::connected(RT::Thread* src,
                              size_t out,
                              RT::Thread* dest,
                              size_t in)
{
  if (!(this->isRegistered(src) && this->isRegistered(dest))) {
    return false;
  }

  auto outputThreadList = this->thread_registry[src][out].output_threads;
  auto outloc = std::find_if(
      outputThreadList.begin(),
      outputThreadList.end(),
      [&dest, &in](thread_connection_t endthread_con)
      { return endthread_con.dest == dest && endthread_con.dest_port == in; });
  return !(outloc == outputThreadList.end());
}

bool RT::Connector::connected(RT::Thread* src,
                              size_t out,
                              RT::Device* dest,
                              size_t in)
{
  if (!(this->isRegistered(src) && this->isRegistered(dest))) {
    return false;
  }

  // now create connection object and save it
  auto outputDeviceList = this->thread_registry[src][out].output_devices;
  auto outloc = std::find_if(
      outputDeviceList.begin(),
      outputDeviceList.end(),
      [&dest, &in](device_connection_t enddevice_con)
      { return enddevice_con.dest == dest && enddevice_con.dest_port == in; });
  return !(outloc == outputDeviceList.end());
}

bool RT::Connector::connected(RT::Device* src,
                              size_t out,
                              RT::Thread* dest,
                              size_t in)
{
  if (!(this->isRegistered(src) && this->isRegistered(dest))) {
    return false;
  }

  // now create connection object and save it
  auto outputThreadList = this->device_registry[src][out].output_threads;
  auto outloc = std::find_if(
      outputThreadList.begin(),
      outputThreadList.end(),
      [&dest, &in](thread_connection_t endthread_con)
      { return endthread_con.dest == dest && endthread_con.dest_port == in; });
  return !(outloc == outputThreadList.end());
}

bool RT::Connector::connected(RT::Device* src,
                              size_t out,
                              RT::Device* dest,
                              size_t in)
{
  if (!(this->isRegistered(src) && this->isRegistered(dest))) {
    return false;
  }
  // now create connection object and save it
  auto outputDeviceList = this->device_registry[src][out].output_devices;
  auto outloc = std::find_if(
      outputDeviceList.begin(),
      outputDeviceList.end(),
      [&dest, &in](device_connection_t enddevice_con)
      { return enddevice_con.dest == dest && enddevice_con.dest_port == in; });
  return !(outloc == outputDeviceList.end());
}

void RT::Connector::disconnect(RT::Thread* src,
                               size_t out,
                               RT::Thread* dest,
                               size_t in)
{
  if (!(this->isRegistered(src) && this->isRegistered(dest))) {
    return;
  }
  if (!(this->connected(src, out, dest, in))) {
    return;
  }
  auto loc = std::find_if(
      this->thread_registry[src][out].output_threads.begin(),
      this->thread_registry[src][out].output_threads.end(),
      [&dest, &in](RT::thread_connection_t endthread_con)
      { return endthread_con.dest == dest && endthread_con.dest_port == in; });
  if (loc == this->thread_registry[src][out].output_threads.end()) {
    return;
  }
  this->thread_registry[src][out].output_threads.erase(loc);
}

void RT::Connector::disconnect(RT::Thread* src,
                               size_t out,
                               RT::Device* dest,
                               size_t in)
{
  if (!(this->isRegistered(src) && this->isRegistered(dest))) {
    return;
  }
  if (!(this->connected(src, out, dest, in))) {
    return;
  }
  auto loc = std::find_if(
      this->thread_registry[src][out].output_devices.begin(),
      this->thread_registry[src][out].output_devices.end(),
      [&dest, &in](RT::device_connection_t enddevice_con)
      { return enddevice_con.dest == dest && enddevice_con.dest_port == in; });
  if (loc == this->thread_registry[src][out].output_devices.end()) {
    return;
  }
  this->thread_registry[src][out].output_devices.erase(loc);
}

void RT::Connector::disconnect(RT::Device* src,
                               size_t out,
                               RT::Thread* dest,
                               size_t in)
{
  if (!(this->isRegistered(src) || this->isRegistered(dest))) {
    return;
  }
  if (!(this->connected(src, out, dest, in))) {
    return;
  }
  auto loc = std::find_if(
      this->device_registry[src][out].output_threads.begin(),
      this->device_registry[src][out].output_threads.end(),
      [&dest, &in](RT::thread_connection_t endthread_con)
      { return endthread_con.dest == dest && endthread_con.dest_port == in; });
  if (loc == this->device_registry[src][out].output_threads.end()) {
    return;
  }
  this->device_registry[src][out].output_threads.erase(loc);
}

void RT::Connector::disconnect(RT::Device* src,
                               size_t out,
                               RT::Device* dest,
                               size_t in)
{
  if (!(this->isRegistered(src) && this->isRegistered(dest))) {
    return;
  }
  if (!(this->connected(src, out, dest, in))) {
    return;
  }
  auto loc = std::find_if(
      this->device_registry[src][out].output_devices.begin(),
      this->device_registry[src][out].output_devices.end(),
      [&dest, &in](RT::device_connection_t enddevice_con)
      { return enddevice_con.dest == dest && enddevice_con.dest_port == in; });
  if (loc == this->device_registry[src][out].output_devices.end()) {
    return;
  }
  this->device_registry[src][out].output_devices.erase(loc);
}

void RT::Connector::insertBlock(RT::Thread* thread)
{
  if (thread == nullptr || this->isRegistered(thread)) {
    return;
  }
  auto n_output_channels = thread->getCount(IO::OUTPUT);
  this->thread_registry[thread] =
      std::vector<RT::outputs_info>(n_output_channels);
}

void RT::Connector::insertBlock(RT::Device* device)
{
  if (device == nullptr || this->isRegistered(device)) {
    return;
  }
  auto n_output_channels = device->getCount(IO::OUTPUT);
  this->device_registry[device] =
      std::vector<RT::outputs_info>(n_output_channels);
}

void RT::Connector::removeBlock(RT::Thread* thread)
{
  if (thread == nullptr || !(this->isRegistered(thread))) {
    return;
  }
  this->thread_registry.erase(thread);
}

void RT::Connector::removeBlock(RT::Device* device)
{
  if (device == nullptr || !(this->isRegistered(device))) {
    return;
  }
  this->device_registry.erase(device);
}

bool RT::Connector::isRegistered(RT::Thread* thread)
{
  return !(this->thread_registry.find(thread) == this->thread_registry.end());
}

bool RT::Connector::isRegistered(RT::Device* device)
{
  return !(this->device_registry.find(device) == this->device_registry.end());
}

std::vector<RT::Thread*> RT::Connector::topological_sort()
{
  auto processing_q = std::queue<RT::Thread*>();
  auto sorted_blocks = std::vector<RT::Thread*>();
  auto sources_per_block = std::unordered_map<RT::Thread*, int>();

  // initialize counts
  for (const auto& block : this->thread_registry) {
    sources_per_block[block.first] = 0;
  }

  // Calculate number of sources per block
  for (const auto& outputs : this->thread_registry) {
    for (const auto& destination_con : outputs.second) {
      for (auto dest_thread : destination_con.output_threads) {
        sources_per_block[dest_thread.dest] += 1;
      }
    }
  }

  // Initialize queue for processing nodes in graph
  for (auto block_count : sources_per_block) {
    if (block_count.second == 0) {
      processing_q.push(block_count.first);
    }
  }

  // Process the graph nodes.
  while (!processing_q.empty()) {
    sorted_blocks.push_back(processing_q.front());
    processing_q.pop();
    for (const auto& connections : this->thread_registry[sorted_blocks.back()])
    {
      for (auto endthread_con : connections.output_threads) {
        sources_per_block[endthread_con.dest] -= 1;
        if (sources_per_block[endthread_con.dest] == 0) {
          processing_q.push(endthread_con.dest);
        }
      }
    }
  }

  return sorted_blocks;
}

std::vector<RT::Device*> RT::Connector::getDevices()
{
  std::vector<RT::Device*> devices;
  for (const auto& block_info : this->device_registry) {
    devices.push_back(block_info.first);
  }
  return devices;
}

std::vector<RT::Thread*> RT::Connector::getThreads()
{
  return this->topological_sort();
}

std::vector<RT::outputs_info> RT::Connector::getOutputs(RT::Thread* src)
{
  auto result = std::vector<RT::outputs_info>();
  if (this->isRegistered(src)) {
    result = this->thread_registry[src];
  }
  return result;
}

std::vector<RT::outputs_info> RT::Connector::getOutputs(RT::Device* src)
{
  auto result = std::vector<RT::outputs_info>();
  if (this->isRegistered(src)) {
    result = this->device_registry[src];
  }
  return result;
}

void RT::Connector::propagateDeviceConnections(RT::Device* device)
{
  for (size_t out_ch = 0; out_ch < this->device_registry[device].size();
       out_ch++) {
    for (auto dest_info : this->device_registry[device][out_ch].output_devices)
    {
      dest_info.dest->writeinput(dest_info.dest_port,
                                 device->readoutput(out_ch));
    }
    for (auto dest_info : this->device_registry[device][out_ch].output_threads)
    {
      dest_info.dest->writeinput(dest_info.dest_port,
                                 device->readoutput(out_ch));
    }
  }
}

void RT::Connector::propagateThreadConnections(RT::Thread* thread)
{
  for (size_t out_ch = 0; out_ch < this->thread_registry[thread].size();
       out_ch++) {
    for (auto& dest_info : this->thread_registry[thread][out_ch].output_devices)
    {
      dest_info.dest->writeinput(dest_info.dest_port,
                                 thread->readoutput(out_ch));
    }
    for (auto& dest_info : this->thread_registry[thread][out_ch].output_threads)
    {
      dest_info.dest->writeinput(dest_info.dest_port,
                                 thread->readoutput(out_ch));
    }
  }
}

RT::System::System(Event::Manager* em, RT::Connector* rtc)
    : event_manager(em)
    , rt_connector(rtc)
{
  if (RT::OS::getFifo(this->eventFifo,
                      RT::OS::DEFAULT_FIFO_SIZE * sizeof(Event::Object*))
      != 0)
  {
    ERROR_MSG("RT::System::System : failed to create Fifo");
    return;
  }
  this->task = std::make_unique<RT::OS::Task>();
  if (RT::OS::createTask(this->task.get(), &RT::System::execute, this) != 0) {
    ERROR_MSG("RT::System::System : failed to create realtime thread\n");
    return;
  }

  this->event_manager->registerHandler(this);
}

RT::System::~System()
{
  this->task->task_finished = true;
  RT::OS::deleteTask(this->task.get());
  this->event_manager->unregisterHandler(this);
}

int64_t RT::System::getPeriod()
{
  return this->task->period;
}

void RT::System::postTelemitry(RT::Telemitry::Response& telemitry)
{
  this->eventFifo->writeRT(&telemitry, sizeof(RT::Telemitry::Response));
}

RT::Telemitry::Response RT::System::getTelemitry()
{
  RT::Telemitry::Response telemitry = RT::Telemitry::NO_TELEMITRY;
  this->eventFifo->read(&telemitry, sizeof(RT::Telemitry::Response));
  return telemitry;
}

void RT::System::setPeriod(RT::System::CMD* cmd)
{
  auto period = std::any_cast<int64_t>(cmd->getParam("period"));
  this->task->period = period;
  auto telem = RT::Telemitry::RT_PERIOD_UPDATE;
  this->postTelemitry(telem);
  cmd->done();
}

void RT::System::updateDeviceList(RT::System::CMD* cmd)
{
  this->devices =
      std::any_cast<std::vector<RT::Device*>>(cmd->getParam("deviceList"));
  auto telem = RT::Telemitry::RT_DEVICE_LIST_UPDATE;
  this->postTelemitry(telem);
  cmd->done();
}

void RT::System::updateThreadList(RT::System::CMD* cmd)
{
  this->threads =
      std::any_cast<std::vector<RT::Thread*>>(cmd->getParam("threadList"));
  auto telem = RT::Telemitry::RT_THREAD_LIST_UPDATE;
  this->postTelemitry(telem);
  cmd->done();
}

void RT::System::updateBlockActivity(RT::System::CMD* cmd)
{
  auto block = std::any_cast<IO::Block*>(cmd->getParam("block"));
  switch (cmd->getType()) {
    case Event::Type::RT_BLOCK_PAUSE_EVENT:
      block->setActive(false);
      cmd->done();
      break;
    case Event::Type::RT_BLOCK_UNPAUSE_EVENT:
      block->setActive(true);
      cmd->done();
      break;
    default:
      break;
  }
}

void RT::System::getPeriodTicksCMD(RT::System::CMD* cmd)
{
  switch (cmd->getType()) {
    case Event::Type::RT_PREPERIOD_EVENT:
      cmd->setParam("pre-period", std::any(&(this->periodStartTime)));
      break;
    case Event::Type::RT_POSTPERIOD_EVENT:
      cmd->setParam("post-period", std::any(&(this->periodEndTime)));
      break;
    default:
      return;
  }
  cmd->done();
}

void RT::System::changeModuleParametersCMD(RT::System::CMD* cmd)
{
  auto* component =
      std::any_cast<Modules::Component*>(cmd->getParam("paramModule"));
  auto param_id = std::any_cast<size_t>(cmd->getParam("paramID"));
  auto param_type =
      std::any_cast<Modules::Variable::variable_t>(cmd->getParam("paramType"));
  std::any param_value_any = cmd->getParam("paramValue");
  switch (param_type) {
    case Modules::Variable::DOUBLE_PARAMETER:
      component->setValue<double>(param_id,
                                  std::any_cast<double>(param_value_any));
      break;
    case Modules::Variable::INT_PARAMETER:
      component->setValue<int64_t>(param_id,
                                   std::any_cast<int64_t>(param_value_any));
      break;
    case Modules::Variable::UINT_PARAMETER:
      component->setValue<uint64_t>(param_id,
                                    std::any_cast<uint64_t>(param_value_any));
      break;
    case Modules::Variable::STATE:
      component->setValue<Modules::Variable::state_t>(
          param_id,
          std::any_cast<Modules::Variable::state_t>(param_value_any));
      break;
    default:
      ERROR_MSG(
          "Module Parameter Change event does not contain expected parameter "
          "types");
  }
  cmd->done();
}

void RT::System::executeCMD(RT::System::CMD* cmd)
{
  RT::Telemitry::Response telem = RT::Telemitry::NO_TELEMITRY;
  switch (cmd->getType()) {
    case Event::Type::RT_PERIOD_EVENT:
      this->setPeriod(cmd);
      break;
    case Event::Type::RT_PREPERIOD_EVENT:
    case Event::Type::RT_POSTPERIOD_EVENT:
      this->getPeriodTicksCMD(cmd);
      break;
    case Event::Type::RT_DEVICE_INSERT_EVENT:
    case Event::Type::RT_DEVICE_REMOVE_EVENT:
      this->updateDeviceList(cmd);
      break;
    case Event::Type::RT_THREAD_INSERT_EVENT:
    case Event::Type::RT_THREAD_REMOVE_EVENT:
      this->updateThreadList(cmd);
      break;
    case Event::Type::RT_BLOCK_PAUSE_EVENT:
    case Event::Type::RT_BLOCK_UNPAUSE_EVENT:
      this->updateBlockActivity(cmd);
      break;
    case Event::Type::RT_SHUTDOWN_EVENT:
      this->task->task_finished = true;
      telem = RT::Telemitry::RT_SHUTDOWN;
      this->postTelemitry(telem);
      cmd->done();
      break;
    case Event::Type::RT_MODULE_PARAMETER_CHANGE_EVENT:
      this->changeModuleParametersCMD(cmd);
      break;
    case Event::Type::NOOP:
      telem = RT::Telemitry::RT_NOOP;
      this->postTelemitry(telem);
      cmd->done();
      break;
    default:
      telem = RT::Telemitry::RT_ERROR;
      RT::System::postTelemitry(telem);
      // make sure the command is handled so caller can continue
      cmd->done();
  }
}

void RT::System::receiveEvent(Event::Object* event)
{
  switch (event->getType()) {
    case Event::Type::RT_PERIOD_EVENT:
      this->setPeriod(event);
      break;
    case Event::Type::RT_THREAD_INSERT_EVENT:
      this->insertThread(event);
      break;
    case Event::Type::RT_THREAD_REMOVE_EVENT:
      this->removeThread(event);
      break;
    case Event::Type::RT_BLOCK_PAUSE_EVENT:
    case Event::Type::RT_BLOCK_UNPAUSE_EVENT:
      this->blockActivityChange(event);
      break;
    case Event::Type::RT_DEVICE_INSERT_EVENT:
      this->insertDevice(event);
      break;
    case Event::Type::RT_DEVICE_REMOVE_EVENT:
      this->removeDevice(event);
      break;
    case Event::Type::RT_MODULE_PARAMETER_CHANGE_EVENT:
      this->changeModuleParameters(event);
      break;
    case Event::Type::RT_SHUTDOWN_EVENT:
      this->shutdown(event);
      break;
    case Event::Type::RT_PREPERIOD_EVENT:
    case Event::Type::RT_POSTPERIOD_EVENT:
      this->provideTimetickPointers(event);
      break;
    case Event::Type::RT_GET_PERIOD_EVENT:
      this->getPeriodValues(event);
      break;
    case Event::Type::NOOP:
      this->NOOP(event);
      break;
    default:
      return;
  }
}

void RT::System::setPeriod(Event::Object* event)
{
  // auto period = std::any_cast<int64_t>(event->getParam("period"));
  RT::System::CMD cmd(*event);
  RT::System::CMD* cmd_ptr = &cmd;
  this->eventFifo->write(&cmd_ptr, sizeof(RT::System::CMD*));
  cmd.wait();
}

void RT::System::getPeriodValues(Event::Object* event)
{
  event->setParam("period", std::any(this->getPeriod()));
}

void RT::System::NOOP(Event::Object* event)
{
  RT::System::CMD cmd(Event::Type::NOOP);
  RT::System::CMD* cmd_ptr = &cmd;
  this->eventFifo->write(&cmd_ptr, sizeof(RT::System::CMD*));
  cmd.wait();
}

void RT::System::insertDevice(Event::Object* event)
{
  auto* device = std::any_cast<RT::Device*>(event->getParam("device"));
  if (device == nullptr) {
    ERROR_MSG("RT::System::insertDevice : invalid device pointer\n");
    return;
  }
  this->rt_connector->insertBlock(device);
  std::vector<RT::Device*> device_list = this->rt_connector->getDevices();
  RT::System::CMD cmd(*event);
  cmd.setParam("deviceList", std::any(device_list));
  RT::System::CMD* cmd_ptr = &cmd;
  this->eventFifo->write(&cmd_ptr, sizeof(RT::System::CMD*));
  cmd.wait();
}

void RT::System::removeDevice(Event::Object* event)
{
  auto* device = std::any_cast<RT::Device*>(event->getParam("device"));
  if (device == nullptr) {
    ERROR_MSG("RT::System::removeDevice : invalid device pointer\n");
    return;
  }
  // We have to make sure to deactivate device before removing
  device->setActive(false);
  this->rt_connector->removeBlock(device);
  std::vector<RT::Device*> device_list = this->rt_connector->getDevices();
  RT::System::CMD cmd(*event);
  cmd.setParam("deviceList", std::any(device_list));
  RT::System::CMD* cmd_ptr = &cmd;
  this->eventFifo->write(&cmd_ptr, sizeof(RT::System::CMD*));
  cmd.wait();
}

void RT::System::insertThread(Event::Object* event)
{
  auto* thread = std::any_cast<RT::Thread*>(event->getParam("thread"));
  if (thread == nullptr) {
    ERROR_MSG("RT::System::removeDevice : invalid device pointer\n");
    return;
  }
  this->rt_connector->insertBlock(thread);
  std::vector<RT::Thread*> thread_list = this->rt_connector->getThreads();
  RT::System::CMD cmd(*event);
  cmd.setParam("threadList", std::any(thread_list));
  RT::System::CMD* cmd_ptr = &cmd;
  this->eventFifo->write(&cmd_ptr, sizeof(RT::System::CMD*));
  cmd.wait();
}

void RT::System::removeThread(Event::Object* event)
{
  auto* thread = std::any_cast<RT::Thread*>(event->getParam("thread"));
  if (thread == nullptr) {
    ERROR_MSG("RT::System::removeDevice : invalid device pointer\n");
    return;
  }
  // We have to make sure to deactivate thread before removing
  thread->setActive(false);
  this->rt_connector->removeBlock(thread);
  std::vector<RT::Thread*> thread_list = this->rt_connector->getThreads();
  RT::System::CMD cmd(*event);
  cmd.setParam("threadList", std::any(thread_list));
  RT::System::CMD* cmd_ptr = &cmd;
  this->eventFifo->write(&cmd_ptr, sizeof(RT::System::CMD*));
  cmd.wait();
}

void RT::System::blockActivityChange(Event::Object* event)
{
  RT::System::CMD cmd(*event);
  cmd.setParam("block", std::any(event->getParam("block")));
  RT::System::CMD* cmd_ptr = &cmd;

  this->eventFifo->write(&cmd_ptr, sizeof(RT::System::CMD*));
  cmd.wait();
}

void RT::System::shutdown(Event::Object* event)
{
  RT::System::CMD cmd(*event);
  RT::System::CMD* cmd_ptr = &cmd;
  this->eventFifo->write(&cmd_ptr, sizeof(RT::System::CMD*));
  cmd.wait();
}

void RT::System::provideTimetickPointers(Event::Object* event)
{
  RT::System::CMD cmd(*event);
  RT::System::CMD* cmd_ptr = &cmd;
  this->eventFifo->write(&cmd_ptr, sizeof(RT::System::CMD*));
  cmd.wait();

  int64_t* startperiod = nullptr;
  int64_t* stopperiod = nullptr;
  // transfer values to event for poster to use
  switch (event->getType()) {
    case Event::Type::RT_PREPERIOD_EVENT:
      startperiod = std::any_cast<int64_t*>(cmd.getParam("pre-period"));
      event->setParam("pre-period", std::any(startperiod));
      break;
    case Event::Type::RT_POSTPERIOD_EVENT:
      stopperiod = std::any_cast<int64_t*>(cmd.getParam("post-period"));
      event->setParam("post-period", std::any(stopperiod));
      break;
    default:
      return;
  }
}

void RT::System::changeModuleParameters(Event::Object* event)
{
  // We will just dynamic cast since we do not make nay changes to the
  // event parameters themeselves
  RT::System::CMD cmd(*event);
  RT::System::CMD* cmd_ptr = &cmd;
  this->eventFifo->write(&cmd_ptr, sizeof(RT::System::CMD*));
  cmd_ptr->wait();
}

void RT::System::execute(void* sys)
{
  auto* system = static_cast<RT::System*>(sys);
  RT::System::CMD* cmd = nullptr;

  if (RT::OS::setPeriod(system->task.get(), RT::OS::DEFAULT_PERIOD) != 0) {
    ERROR_MSG(
        "RT::System::execute : failed to set the initial period of the "
        "realtime thread\n");
    return;
  }
  auto starttime = RT::OS::getTime();
  int64_t endtime = 0;
  while (!(system->task->task_finished)) {
    // storing timing information and placing it in local variables

    // store period timing values for previous period
    system->periodStartTime = starttime;
    system->periodEndTime = endtime;

    // sleep until next cycle
    RT::OS::sleepTimestep(system->task.get());
    starttime = RT::OS::getTime();

    for (auto* iDevice : system->devices) {
      if (iDevice->getActive()) {
        iDevice->read();
        system->rt_connector->propagateDeviceConnections(iDevice);
      }
    }

    for (auto* iThread : system->threads) {
      if (iThread->getActive()) {
        iThread->execute();
        system->rt_connector->propagateThreadConnections(iThread);
      }
    }
    endtime = RT::OS::getTime();

    for (auto* iDevice : system->devices) {
      if (iDevice->getActive()) {
        iDevice->write();
      }
    }

    while (system->eventFifo->readRT(&cmd, sizeof(RT::System::CMD*)) != -1) {
      system->executeCMD(cmd);
    }
    cmd = nullptr;
  }
}
