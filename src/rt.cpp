/*
         The Real-Time eXperiment Interface (RTXI)
         Copyright (C) 2011 Georgia Institute of Technology, University of Utah,
   Will Cornell Medical College

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

#include <functional>
#include <queue>

#include "rt.hpp"

#include "debug.hpp"
#include "event.hpp"
#include "fifo.hpp"
#include "module.hpp"
#include "rtos.hpp"

void RT::Device::read()
{
  this->read_callback();
}

void RT::Device::write()
{
  this->write_callback();
}

void RT::Device::bind_read_callback(std::function<void(void)> callback)
{
  this->read_callback = std::move(callback);
}

void RT::Device::bind_write_callback(std::function<void(void)> callback)
{
  this->write_callback = std::move(callback);
}

void RT::Thread::execute()
{
  this->execute_callback();
}

void RT::Thread::bind_execute_callback(std::function<void(void)> callback)
{
  this->execute_callback = std::move(callback);
}

int RT::Connector::connect(Thread* src, size_t out, RT::Thread* dest, size_t in)
{
  // Let's remind our users to register their block first
  if (!(this->isRegistered(src) && this->isRegistered(dest))) {
    ERROR_MSG(
        "RT::Connector : source or destination threads are not registered");
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
    for (const auto& connections :
         this->thread_registry[processing.front()->getID()]
             .channels_outbound_con)
    {
      for (auto thread_connection : connections.output_threads) {
        processing.push(thread_connection.dest);
      }
    }
    processing.pop();
  }

  if (!(this->connected(src, out, dest, in))) {
    thread_connection_t tempcon = {dest, in};
    this->thread_registry[src->getID()]
        .channels_outbound_con[out]
        .output_threads.push_back(tempcon);
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
        "RT::Connector : source or destination threads are not registered");
    return -1;
  }

  if (!(this->connected(src, out, dest, in))) {
    device_connection_t tempcon = {dest, in};
    this->thread_registry[src->getID()]
        .channels_outbound_con[out]
        .output_devices.push_back(tempcon);
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
        "RT::Connector : source or destination threads are not registered");
    return -1;
  }

  if (!(this->connected(src, out, dest, in))) {
    thread_connection_t tempcon = {dest, in};
    this->device_registry[src->getID()]
        .channels_outbound_con[out]
        .output_threads.push_back(tempcon);
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
        "RT::Connector : source or destination threads are not registered");
    return -1;
  }

  if (!(this->connected(src, out, dest, in))) {
    device_connection_t tempcon = {dest, in};
    this->device_registry[src->getID()]
        .channels_outbound_con[out]
        .output_devices.push_back(tempcon);
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

  auto outputThreadList = this->thread_registry[src->getID()]
                              .channels_outbound_con[out]
                              .output_threads;
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
  auto outputDeviceList = this->thread_registry[src->getID()]
                              .channels_outbound_con[out]
                              .output_devices;
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
  auto outputThreadList = this->device_registry[src->getID()]
                              .channels_outbound_con[out]
                              .output_threads;
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
  auto outputDeviceList = this->device_registry[src->getID()]
                              .channels_outbound_con[out]
                              .output_devices;
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
  auto src_id = src->getID();
  auto loc = std::find_if(
      this->thread_registry[src_id]
          .channels_outbound_con[out]
          .output_threads.begin(),
      this->thread_registry[src_id]
          .channels_outbound_con[out]
          .output_threads.end(),
      [&dest, &in](RT::thread_connection_t endthread_con)
      { return endthread_con.dest == dest && endthread_con.dest_port == in; });
  if (loc
      == this->thread_registry[src_id]
             .channels_outbound_con[out]
             .output_threads.end())
  {
    return;
  }
  this->thread_registry[src_id].channels_outbound_con[out].output_threads.erase(
      loc);
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
  auto src_id = src->getID();
  auto loc = std::find_if(
      this->thread_registry[src_id]
          .channels_outbound_con[out]
          .output_devices.begin(),
      this->thread_registry[src_id]
          .channels_outbound_con[out]
          .output_devices.end(),
      [&dest, &in](RT::device_connection_t enddevice_con)
      { return enddevice_con.dest == dest && enddevice_con.dest_port == in; });
  if (loc
      == this->thread_registry[src_id]
             .channels_outbound_con[out]
             .output_devices.end())
  {
    return;
  }
  this->thread_registry[src_id].channels_outbound_con[out].output_devices.erase(
      loc);
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
  auto src_id = src->getID();
  auto loc = std::find_if(
      this->device_registry[src_id]
          .channels_outbound_con[out]
          .output_threads.begin(),
      this->device_registry[src_id]
          .channels_outbound_con[out]
          .output_threads.end(),
      [&dest, &in](RT::thread_connection_t endthread_con)
      { return endthread_con.dest == dest && endthread_con.dest_port == in; });
  if (loc
      == this->device_registry[src_id]
             .channels_outbound_con[out]
             .output_threads.end())
  {
    return;
  }
  this->device_registry[src_id].channels_outbound_con[out].output_threads.erase(
      loc);
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
  auto src_id = src->getID();
  auto loc = std::find_if(
      this->device_registry[src_id]
          .channels_outbound_con[out]
          .output_devices.begin(),
      this->device_registry[src_id]
          .channels_outbound_con[out]
          .output_devices.end(),
      [&dest, &in](RT::device_connection_t enddevice_con)
      { return enddevice_con.dest == dest && enddevice_con.dest_port == in; });
  if (loc
      == this->device_registry[src_id]
             .channels_outbound_con[out]
             .output_devices.end())
  {
    return;
  }
  this->device_registry[src_id].channels_outbound_con[out].output_devices.erase(
      loc);
}

void RT::Connector::insertBlock(RT::Thread* thread)
{
  if (thread == nullptr || this->isRegistered(thread)) {
    return;
  }
  auto n_output_channels = thread->getCount(IO::OUTPUT);
  auto output_channels = std::vector<RT::outputs_info>(n_output_channels);
  size_t newid = 0;
  bool inserted = false;
  for (auto& thread_entry : this->thread_registry) {
    if (thread_entry.thread_ptr == nullptr) {
      thread_entry.thread_ptr = thread;
      thread_entry.channels_outbound_con = output_channels;
      inserted = true;
      thread->assignID(newid);
      break;
    }
    newid++;
  }
  if (!inserted) {
    thread->assignID(this->thread_registry.size());
    this->thread_registry.push_back({thread, output_channels});
  }
}

void RT::Connector::insertBlock(RT::Device* device)
{
  if (device == nullptr || this->isRegistered(device)) {
    return;
  }
  auto n_output_channels = device->getCount(IO::OUTPUT);
  auto output_channels = std::vector<RT::outputs_info>(n_output_channels);
  size_t newid = 0;
  bool inserted = false;
  for (auto& device_entry : this->device_registry) {
    if (device_entry.device_ptr == nullptr) {
      device_entry.device_ptr = device;
      device_entry.channels_outbound_con = output_channels;
      inserted = true;
      device->assignID(newid);
      break;
    }
    newid++;
  }
  if (!inserted) {
    device->assignID(this->device_registry.size());
    this->device_registry.push_back({device, output_channels});
  }
}

void RT::Connector::removeBlock(RT::Thread* thread)
{
  if (thread == nullptr || !(this->isRegistered(thread))) {
    return;
  }
  this->thread_registry[thread->getID()] = {nullptr,
                                            std::vector<RT::outputs_info>()};
}

void RT::Connector::removeBlock(RT::Device* device)
{
  if (device == nullptr || !(this->isRegistered(device))) {
    return;
  }
  this->device_registry[device->getID()] = {nullptr,
                                            std::vector<RT::outputs_info>()};
}

bool RT::Connector::isRegistered(RT::Thread* thread)
{
  return thread->getID() < this->thread_registry.size()
      && this->thread_registry[thread->getID()].thread_ptr == thread;
}

bool RT::Connector::isRegistered(RT::Device* device)
{
  return device->getID() < this->device_registry.size()
      && this->device_registry[device->getID()].device_ptr == device;
}

std::vector<RT::Thread*> RT::Connector::topological_sort()
{
  auto processing_q = std::queue<RT::Thread*>();
  auto sorted_blocks = std::vector<RT::Thread*>();
  auto sources_per_block = std::unordered_map<RT::Thread*, int>();
  auto valid_threads = std::vector<thread_entry_t>();

  // initialize counts
  for (const auto& block : this->thread_registry) {
    if (block.thread_ptr == nullptr) {
      continue;
    }
    sources_per_block[block.thread_ptr] = 0;
  }

  // Calculate number of sources per block
  for (const auto& outputs : this->thread_registry) {
    if (outputs.thread_ptr == nullptr) {
      continue;
    }
    for (const auto& destination_con : outputs.channels_outbound_con) {
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
    auto block_id = sorted_blocks.back()->getID();
    for (const auto& connections :
         this->thread_registry[block_id].channels_outbound_con)
    {
      for (auto endthread_con : connections.output_threads) {
        sources_per_block[endthread_con.dest] -= 1;
        if (sources_per_block[endthread_con.dest] == 0) {
          processing_q.push(endthread_con.dest);
        }
      }
    }
  }

  // System only cares about active blocks
  std::vector<RT::Thread*> sorted_active_blocks;
  for (auto& active_thread : sorted_blocks) {
    if (active_thread->getActive()) {
      sorted_active_blocks.push_back(active_thread);
    }
  }
  return sorted_active_blocks;
}

std::vector<RT::Device*> RT::Connector::getDevices()
{
  std::vector<RT::Device*> devices;
  for (const auto& block_info : this->device_registry) {
    if (block_info.device_ptr != nullptr && block_info.device_ptr->getActive()) {
      devices.push_back(block_info.device_ptr);
    }
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
    result = this->thread_registry[src->getID()].channels_outbound_con;
  }
  return result;
}

std::vector<RT::outputs_info> RT::Connector::getOutputs(RT::Device* src)
{
  auto result = std::vector<RT::outputs_info>();
  if (this->isRegistered(src)) {
    result = this->device_registry[src->getID()].channels_outbound_con;
  }
  return result;
}

void RT::Connector::propagateDeviceConnections(RT::Device* device)
{
  auto dev_id = device->getID();
  for (size_t out_ch = 0;
       out_ch < this->device_registry[dev_id].channels_outbound_con.size();
       out_ch++)
  {
    for (auto dest_info : this->device_registry[dev_id]
                              .channels_outbound_con[out_ch]
                              .output_devices)
    {
      dest_info.dest->writeinput(dest_info.dest_port,
                                 device->readoutput(out_ch));
    }
    for (auto dest_info : this->device_registry[dev_id]
                              .channels_outbound_con[out_ch]
                              .output_threads)
    {
      dest_info.dest->writeinput(dest_info.dest_port,
                                 device->readoutput(out_ch));
    }
  }
}

void RT::Connector::propagateThreadConnections(RT::Thread* thread)
{
  auto thread_id = thread->getID();
  for (size_t out_ch = 0;
       out_ch < this->thread_registry[thread_id].channels_outbound_con.size();
       out_ch++)
  {
    for (auto& dest_info : this->thread_registry[thread_id]
                               .channels_outbound_con[out_ch]
                               .output_devices)
    {
      dest_info.dest->writeinput(dest_info.dest_port,
                                 thread->readoutput(out_ch));
    }
    for (auto& dest_info : this->thread_registry[thread_id]
                               .channels_outbound_con[out_ch]
                               .output_threads)
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

  this->threads.reserve(100);
  this->devices.reserve(100);
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

void RT::System::postTelemitry(RT::Telemitry::Response telemitry)
{
  this->eventFifo->writeRT(&telemitry, sizeof(RT::Telemitry::Response));
}

void RT::System::createTelemitryProcessor()
{
  auto proc = [&](){
    bool running = true;
    std::vector<RT::Telemitry::Response> responses;
    while(!this->task->task_finished && running){
      responses = this->getTelemitry();
      for(auto telem : responses){
        if(telem.cmd != nullptr) {
          telem.cmd->done();
        }
        if(telem.type == RT::Telemitry::RT_SHUTDOWN) { 
          running = false; 
        }
      }
    }
  };
  std::thread(proc).detach();
}

std::vector<RT::Telemitry::Response> RT::System::getTelemitry()
{
  this->eventFifo->poll();
  std::vector<RT::Telemitry::Response> responses;
  RT::Telemitry::Response telemitry; 
  while(this->eventFifo->read(&telemitry, sizeof(RT::Telemitry::Response)) > 0){
    responses.push_back(telemitry);
  }
  return responses;
}

void RT::System::setPeriod(RT::System::CMD* cmd)
{
  auto period = std::get<int64_t>(cmd->getRTParam("period"));
  this->task->period = period;
  RT::Telemitry::Response telem = {
    RT::Telemitry::RT_PERIOD_UPDATE,
    cmd
  };
  this->postTelemitry(telem);
  
}

void RT::System::updateDeviceList(RT::System::CMD* cmd)
{
  std::vector<RT::Device*>* vec_ptr = std::get<std::vector<RT::Device*>*>(cmd->getRTParam("deviceList"));
  this->devices.clear();
  this->devices.assign(vec_ptr->begin(), vec_ptr->end());
  RT::Telemitry::Response telem = {
    RT::Telemitry::RT_DEVICE_LIST_UPDATE,
    cmd
  };
  this->postTelemitry(telem);
  
}

void RT::System::updateThreadList(RT::System::CMD* cmd)
{
  std::vector<RT::Thread*>* vec_ptr = std::get<std::vector<RT::Thread*>*>(cmd->getRTParam("threadList"));
  this->threads.clear();
  this->threads.assign(vec_ptr->begin(), vec_ptr->end());
  RT::Telemitry::Response telem = {
    RT::Telemitry::RT_THREAD_LIST_UPDATE,
    cmd
  };
  this->postTelemitry(telem);
  
}

// void RT::System::updateBlockActivity(RT::System::CMD* cmd)
//{
//   auto block = std::any_cast<IO::Block*>(cmd->getParam("block"));
//   switch (cmd->getType()) {
//     case Event::Type::RT_BLOCK_PAUSE_EVENT:
//       block->setActive(false);
//       
//       break;
//     case Event::Type::RT_BLOCK_UNPAUSE_EVENT:
//       block->setActive(true);
//       
//       break;
//     default:
//       break;
//   }
// }

void RT::System::getPeriodTicksCMD(RT::System::CMD* cmd)
{
  RT::command_param_t value;
  switch (cmd->getType()) {
    case Event::Type::RT_PREPERIOD_EVENT:
      value = &(this->periodStartTime);
      cmd->setRTParam("pre-period", value);
      break;
    case Event::Type::RT_POSTPERIOD_EVENT:
      value = &(this->periodEndTime);
      cmd->setRTParam("post-period", value);
      break;
    default:
      return;
  }
  RT::Telemitry::Response telem = {
    RT::Telemitry::RT_PERIOD_UPDATE,
    cmd
  };
  this->postTelemitry(telem);
}

void RT::System::changeModuleParametersCMD(RT::System::CMD* cmd)
{
  RT::Telemitry::Response telem;
  telem.cmd = cmd;
  telem.type = RT::Telemitry::RT_MODULE_PARAM_UPDATE;
  auto* component =
      std::get<Modules::Component*>(cmd->getRTParam("paramModule"));
  auto param_id = std::get<size_t>(cmd->getRTParam("paramID"));
  auto param_type_num = std::get<uint64_t>(cmd->getRTParam("paramType"));
  auto param_type = static_cast<Modules::Variable::variable_t>(param_type_num);
  RT::command_param_t param_value_any = cmd->getRTParam("paramValue");
  switch (param_type) {
    case Modules::Variable::DOUBLE_PARAMETER:
      component->setValue<double>(param_id,
                                  std::get<double>(param_value_any));
      break;
    case Modules::Variable::INT_PARAMETER:
      component->setValue<int64_t>(param_id,
                                   std::get<int64_t>(param_value_any));
      break;
    case Modules::Variable::UINT_PARAMETER:
      component->setValue<uint64_t>(param_id,
                                    std::get<uint64_t>(param_value_any));
      break;
    case Modules::Variable::STATE:
      component->setValue<Modules::Variable::state_t>(
          param_id, 
          static_cast<Modules::Variable::state_t>(std::get<int64_t>(param_value_any)));
      break;
    default:
      ERROR_MSG(
          "Module Parameter Change event does not contain expected parameter "
          "types");
      telem.type = RT::Telemitry::RT_ERROR;
  }
  this->postTelemitry(telem);
}

void RT::System::executeCMD(RT::System::CMD* cmd)
{
  RT::Telemitry::Response telem;
  telem.cmd = cmd;
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
    case Event::Type::RT_DEVICE_PAUSE_EVENT:
    case Event::Type::RT_DEVICE_UNPAUSE_EVENT:
      this->updateDeviceList(cmd);
      break;
    case Event::Type::RT_THREAD_INSERT_EVENT:
    case Event::Type::RT_THREAD_REMOVE_EVENT:
    case Event::Type::RT_THREAD_PAUSE_EVENT:
    case Event::Type::RT_THREAD_UNPAUSE_EVENT:
      this->updateThreadList(cmd);
      break;
    case Event::Type::RT_SHUTDOWN_EVENT:
      this->task->task_finished = true;
      telem.type = RT::Telemitry::RT_SHUTDOWN;
      telem.cmd = cmd;
      this->postTelemitry(telem);
      break;
    case Event::Type::RT_MODULE_PARAMETER_CHANGE_EVENT:
      this->changeModuleParametersCMD(cmd);
      break;
    case Event::Type::NOOP:
      telem.type = RT::Telemitry::RT_NOOP;
      telem.cmd = cmd;
      this->postTelemitry(telem);
      break;
    default:
      telem.type = RT::Telemitry::RT_ERROR;
      telem.cmd = nullptr;
      RT::System::postTelemitry(telem);
      // make sure the command is handled so caller can continue
  }
}

void RT::System::receiveEvent(Event::Object* event)
{
  // funnily enough it may be that real-time loop
  // is already shutting down before unregistering 
  // system from the list of event handlers. 
  // In that case we shouldn't attempt to send commands 
  // or process events
  if(this->task->task_finished) { return; }

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
    case Event::Type::RT_THREAD_PAUSE_EVENT:
    case Event::Type::RT_THREAD_UNPAUSE_EVENT:
      this->threadActivityChange(event);
      break;
    case Event::Type::RT_DEVICE_PAUSE_EVENT:
    case Event::Type::RT_DEVICE_UNPAUSE_EVENT:
      this->deviceActivityChange(event);
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
  RT::System::CMD cmd(event->getType());
  auto period = std::any_cast<int64_t>(event->getParam("period"));
  cmd.setRTParam("period", period);
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
  RT::System::CMD cmd(event->getType());
  cmd.setRTParam("deviceList", &device_list);
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
  RT::System::CMD cmd(event->getType());
  cmd.setRTParam("deviceList", &device_list);
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
  RT::System::CMD cmd(event->getType());
  cmd.setRTParam("threadList", &thread_list);
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
  RT::System::CMD cmd(event->getType());
  cmd.setRTParam("threadList", &thread_list);
  RT::System::CMD* cmd_ptr = &cmd;
  this->eventFifo->write(&cmd_ptr, sizeof(RT::System::CMD*));
  cmd.wait();
}

void RT::System::threadActivityChange(Event::Object* event)
{
  auto isactive = event->getType() == Event::Type::RT_THREAD_UNPAUSE_EVENT;
  RT::System::CMD cmd(event->getType());
  RT::System::CMD* cmd_ptr = &cmd;
  auto* thread = std::any_cast<RT::Thread*>(event->getParam("thread"));
  thread->setActive(isactive);
  auto thread_list = this->rt_connector->getThreads();
  cmd.setRTParam("threadList", &thread_list);
  this->eventFifo->write(&cmd_ptr, sizeof(RT::System::CMD*));
  cmd.wait();
}

void RT::System::deviceActivityChange(Event::Object* event)
{
  auto isactive = event->getType() == Event::Type::RT_DEVICE_UNPAUSE_EVENT;
  RT::System::CMD cmd(event->getType());
  RT::System::CMD* cmd_ptr = &cmd;
  auto* device = std::any_cast<RT::Device*>(event->getParam("device"));
  device->setActive(isactive);
  auto device_list = this->rt_connector->getDevices();
  cmd.setRTParam("deviceList", &device_list);
  this->eventFifo->write(&cmd_ptr, sizeof(RT::System::CMD*));
  cmd.wait();
}

void RT::System::shutdown(Event::Object* event)
{
  RT::System::CMD cmd(event->getType());
  RT::System::CMD* cmd_ptr = &cmd;
  this->eventFifo->write(&cmd_ptr, sizeof(RT::System::CMD*));
  cmd.wait();
}

void RT::System::provideTimetickPointers(Event::Object* event)
{
  RT::System::CMD cmd(event->getType());
  RT::System::CMD* cmd_ptr = &cmd;
  this->eventFifo->write(&cmd_ptr, sizeof(RT::System::CMD*));
  cmd.wait();

  int64_t* startperiod = nullptr;
  int64_t* stopperiod = nullptr;
  // transfer values to event for poster to use
  switch (event->getType()) {
    case Event::Type::RT_PREPERIOD_EVENT:
      startperiod = std::get<int64_t*>(cmd.getRTParam("pre-period"));
      event->setParam("pre-period", std::any(startperiod));
      break;
    case Event::Type::RT_POSTPERIOD_EVENT:
      stopperiod = std::get<int64_t*>(cmd.getRTParam("post-period"));
      event->setParam("post-period", std::any(stopperiod));
      break;
    default:
      return;
  }
}

void RT::System::changeModuleParameters(Event::Object* event)
{
  // we must convert event object to cmd object
  RT::System::CMD cmd(event->getType());
  auto* component = std::any_cast<Modules::Component*>(event->getParam("paramModule"));
  cmd.setRTParam("paramModule", component);
  auto param_id = std::any_cast<size_t>(event->getParam("paramID"));
  cmd.setRTParam("paramID", param_id);
  auto param_type = std::any_cast<Modules::Variable::variable_t>(event->getParam("paramType"));
  cmd.setRTParam("paramType", param_type);
  std::any param_value_any = event->getParam("paramValue");
  switch (param_type) {
    case Modules::Variable::DOUBLE_PARAMETER:
      cmd.setRTParam("paramValue", 
                     std::any_cast<double>(param_value_any));
      break;
    case Modules::Variable::INT_PARAMETER:
      cmd.setRTParam("paramValue", 
                     std::any_cast<int64_t>(param_value_any));

      break;
    case Modules::Variable::UINT_PARAMETER:
      cmd.setRTParam("paramValue", 
                     std::any_cast<uint64_t>(param_value_any));
      break;
    case Modules::Variable::STATE:
      cmd.setRTParam("paramValue", 
                     std::any_cast<Modules::Variable::state_t>(param_value_any));
      break;
    default:
      ERROR_MSG(
          "Module Parameter Change event does not contain expected parameter "
          "types");
  }

  RT::System::CMD* cmd_ptr = &cmd;
  this->eventFifo->write(&cmd_ptr, sizeof(RT::System::CMD*));
  cmd_ptr->wait();
}

RT::System::CMD::CMD(Event::Type et) : Event::Object(et)
{
}

RT::command_param_t RT::System::CMD::getRTParam(const std::string_view& param_name)
{
  for (auto& parameter : rt_params) {
    if (parameter.name == param_name) {
      return parameter.value;
    }
  }
  return std::monostate();
}

void RT::System::CMD::setRTParam(const std::string_view& param_name, 
                                 const RT::command_param_t& value)
{
  for (auto& parameter : rt_params) {
    if (parameter.name == param_name) {
      parameter.value = value;
      return;
    }
  }

  rt_param temp = {};
  temp.name = param_name;
  temp.value = value;
  rt_params.push_back(temp);
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
      iDevice->read();
      system->rt_connector->propagateDeviceConnections(iDevice);
    }

    for (auto* iThread : system->threads) {
      iThread->execute();
      system->rt_connector->propagateThreadConnections(iThread);
    }

    for (auto* iDevice : system->devices) {
      iDevice->write();
    }

    while (system->eventFifo->readRT(&cmd, sizeof(RT::System::CMD*)) != -1) {
      system->executeCMD(cmd);
    }
    endtime = RT::OS::getTime();
    cmd = nullptr;
  }

}
