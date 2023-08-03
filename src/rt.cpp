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
#include <iostream>
#include <queue>

#include "rt.hpp"

#include "debug.hpp"
#include "event.hpp"
#include "fifo.hpp"
#include "logger.hpp"
#include "module.hpp"
#include "rtos.hpp"

// TODO: convert cycle detection into non-recursive version
int RT::Connector::find_cycle(RT::block_connection_t conn, IO::Block* ref_block)
{
  if (conn.dest == ref_block) {
    return -1;
  }
  for (const auto& temp_conn : this->connections[ref_block->getID()]) {
    if (conn.dest == temp_conn.src
        && this->find_cycle(temp_conn, ref_block) == -1)
    {
      return -1;
    }
  }
  return 0;
}

int RT::Connector::connect(RT::block_connection_t connection)
{
  // Let's remind our users to register their block first
  if (!(this->isRegistered(connection.src)
        && this->isRegistered(connection.dest)))
  {
    ERROR_MSG(
        "RT::Connector : source or destination blocks are not registered");
    return -1;
  }
  if (this->find_cycle(connection, connection.src) == -1) {
    return -1;
  }

  if (!(this->connected(connection))) {
    this->connections[connection.src->getID()].push_back(connection);
  }
  return 0;
}

bool RT::Connector::connected(RT::block_connection_t connection)
{
  if (!(this->isRegistered(connection.src)
        && this->isRegistered(connection.dest)))
  {
    return false;
  }
  size_t src_id = connection.src->getID();
  auto iter = std::find(this->connections[src_id].begin(),
                        this->connections[src_id].end(),
                        connection);
  return iter != this->connections[src_id].end();
}

void RT::Connector::disconnect(RT::block_connection_t connection)
{
  if (!(this->isRegistered(connection.src)
        && this->isRegistered(connection.dest)))
  {
    return;
  }
  size_t src_id = connection.src->getID();
  auto it = std::find(this->connections[src_id].begin(),
                      this->connections[src_id].end(),
                      connection);
  if (it != this->connections[src_id].end()){
    this->connections[src_id].erase(it);
  }
}

void RT::Connector::insertBlock(IO::Block* block, std::vector<RT::block_connection_t>& block_connections)
{
  if (block == nullptr || this->isRegistered(block)) {
    return;
  }

  // This insertion block algorithm makes use of move semantics to insert and delete 
  // vector items. This is mainly to avoid allocations in the real-time thead and
  // std::move allows us to place data without having to allocate memory again.
  // We should make sure to use reserve() memeber function on vectors in the non-rt
  // thread so that further push_back calls are less likely to allocate memory.
  size_t id = 0;
  bool stored = false;
  for (id = 0; id < this->block_registry.size(); id++) {
    if (this->block_registry[id] == nullptr) {
      this->block_registry[id] = block;
      block->assignID(id);
      this->connections[id].swap(block_connections);
      stored = true;
      break;
    }
  }

  if (!stored) {
    block->assignID(this->block_registry.size());
    this->block_registry.push_back(block);
    this->connections.emplace_back(std::move(block_connections));
  }
}

void RT::Connector::removeBlock(IO::Block* block)
{
  if (block == nullptr || !(this->isRegistered(block))) {
    return;
  }
  // remove block from registry
  this->block_registry[block->getID()] = nullptr; 
  //block->assignID(IO::INVALID_BLOCK_ID);
}

bool RT::Connector::isRegistered(IO::Block* block)
{
  if (block == nullptr) {
    return false;
  }
  if (block->getID() >= this->block_registry.size()) {
    return false;
  }
  return block == this->block_registry[block->getID()];
}

std::vector<RT::Thread*> RT::Connector::topological_sort()
{
  auto processing_q = std::queue<IO::Block*>();
  auto sorted_blocks = std::vector<IO::Block*>();
  auto sources_per_block = std::unordered_map<IO::Block*, int>();
  // auto valid_threads = std::vector<IO::Block*>();

  // initialize counts
  for (auto* block : this->block_registry) {
    if (block == nullptr) {
      continue;
    }
    sources_per_block[block] = 0;
  }

  // Calculate number of sources per block
  for (const auto& entry : this->connections) {
    for (const auto& conn : entry) {
      sources_per_block[conn.dest] += 1;
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
    for (const auto& entry : this->connections) {
      for (const auto& conn : entry) {
        if (processing_q.front() == conn.src) {
          sources_per_block[conn.dest] -= 1;
          if (sources_per_block[conn.dest] == 0) {
            processing_q.push(conn.dest);
          }
        }
      }
    }
    processing_q.pop();
  }

  // System only cares about active threads
  std::vector<RT::Thread*> sorted_active_threads;
  for (auto* block : sorted_blocks) {
    if (block->getActive() && block->dependent()) {
      sorted_active_threads.push_back(dynamic_cast<RT::Thread*>(block));
    }
  }
  return sorted_active_threads;
}

std::vector<RT::Device*> RT::Connector::getDevices()
{
  std::vector<RT::Device*> devices;
  for (auto* block : this->block_registry) {
    if (block == nullptr) {
      continue;
    }
    if (block->getActive() && !block->dependent()) {
      devices.push_back(dynamic_cast<RT::Device*>(block));
    }
  }
  return devices;
}

std::vector<RT::Thread*> RT::Connector::getThreads()
{
  return this->topological_sort();
}

std::vector<RT::block_connection_t> RT::Connector::getOutputs(IO::Block* src)
{
  if(!this->isRegistered(src)) { return {}; }
  return this->connections[src->getID()];
}

void RT::Connector::propagateBlockConnections(IO::Block* block)
{
  for (const auto& conn : this->connections[block->getID()]) {
    conn.dest->writeinput(
        conn.dest_port,
        conn.src->readPort(conn.src_port_type, conn.src_port));
  }
}

void RT::Connector::clearAllConnections(IO::Block* block)
{
  for(auto& entry : this->connections){
    entry.erase(std::remove_if(entry.begin(), 
                               entry.end(),
                               [&](RT::block_connection_t conn){
                                 return conn.dest == block;
                               }), entry.end());
  }
}

std::vector<IO::Block*> RT::Connector::getRegisteredBlocks()
{
  std::vector<IO::Block*> blocks;
  for (auto* block : this->block_registry) {
    if (block != nullptr) {
      blocks.push_back(block);
    }
  }
  return blocks;
}

std::vector<RT::block_connection_t> RT::Connector::getAllConnections()
{
  std::vector<RT::block_connection_t> all_connections;
  for (auto entry : this->connections) {
    all_connections.insert(all_connections.end(), entry.begin(), entry.end());
  }
  return all_connections;
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
  this->telemitry_processing_thread_running = false;
  this->eventFifo->close();
  if (this->telemitry_processing_thread.joinable()) {
    this->telemitry_processing_thread.join();
  }
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
  auto proc = [&]()
  {
    eventLogger* logger = this->event_manager->getLogger();
    std::vector<RT::Telemitry::Response> responses;
    while (!this->task->task_finished
           && this->telemitry_processing_thread_running)
    {
      responses = this->getTelemitry();
      for (auto telem : responses) {
        if (telem.cmd != nullptr) {
          telem.cmd->done();
        }
        if (telem.type == RT::Telemitry::RT_SHUTDOWN) {
          this->telemitry_processing_thread_running = false;
        }
        // let's log this telemitry
        logger->log(telem);
      }
    }
  };
  this->telemitry_processing_thread = std::thread(proc);
  RT::OS::renameOSThread(this->telemitry_processing_thread,
                         std::string("TelemitryWorker"));
}

std::vector<RT::Telemitry::Response> RT::System::getTelemitry()
{
  this->eventFifo->poll();
  std::vector<RT::Telemitry::Response> responses;
  RT::Telemitry::Response telemitry;
  while (this->eventFifo->read(&telemitry, sizeof(RT::Telemitry::Response)) > 0)
  {
    responses.push_back(telemitry);
  }
  return responses;
}

void RT::System::setPeriod(RT::System::CMD* cmd)
{
  auto period = std::get<int64_t>(cmd->getRTParam("period"));
  this->task->period = period;
  const RT::Telemitry::Response telem = {RT::Telemitry::RT_PERIOD_UPDATE, cmd};
  this->postTelemitry(telem);
}

void RT::System::updateDeviceList(RT::System::CMD* cmd)
{
  std::vector<RT::Device*>* vec_ptr =
      std::get<std::vector<RT::Device*>*>(cmd->getRTParam("deviceList"));
  this->devices.clear();
  this->devices.assign(vec_ptr->begin(), vec_ptr->end());
  if(cmd->getType() == Event::Type::RT_DEVICE_REMOVE_EVENT){
    auto* device = std::get<RT::Device*>(cmd->getRTParam("device"));
    this->rt_connector->clearAllConnections(device);
    device->assignID(IO::INVALID_BLOCK_ID);
  }
  const RT::Telemitry::Response telem = {RT::Telemitry::RT_DEVICE_LIST_UPDATE,
                                         cmd};
  this->postTelemitry(telem);
}

void RT::System::updateThreadList(RT::System::CMD* cmd)
{
  std::vector<RT::Thread*>* vec_ptr =
      std::get<std::vector<RT::Thread*>*>(cmd->getRTParam("threadList"));
  this->threads.clear();
  this->threads.assign(vec_ptr->begin(), vec_ptr->end());
  if(cmd->getType() == Event::Type::RT_THREAD_REMOVE_EVENT){
    auto* thread = std::get<RT::Thread*>(cmd->getRTParam("thread"));
    this->rt_connector->clearAllConnections(thread);
    thread->assignID(IO::INVALID_BLOCK_ID);
  }
  const RT::Telemitry::Response telem = {RT::Telemitry::RT_THREAD_LIST_UPDATE,
                                         cmd};
  this->postTelemitry(telem);
}

void RT::System::ioLinkUpdateCMD(RT::System::CMD* cmd)
{
  auto conn = std::get<RT::block_connection_t>(cmd->getRTParam("connection"));
  RT::Telemitry::Response telem;
  telem.cmd = cmd;
  switch (cmd->getType()) {
    case Event::Type::IO_LINK_INSERT_EVENT:
      this->rt_connector->connect(conn);
      telem.type = RT::Telemitry::IO_LINK_UPDATED;
      break;
    case Event::Type::IO_LINK_REMOVE_EVENT:
      this->rt_connector->disconnect(conn);
      telem.type = RT::Telemitry::IO_LINK_UPDATED;
      break;
    default:
      telem.type = RT::Telemitry::RT_NOOP;
      break;
  }
  this->postTelemitry(telem);
}

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
  const RT::Telemitry::Response telem = {RT::Telemitry::RT_PERIOD_UPDATE, cmd};
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
      component->setValue<double>(param_id, std::get<double>(param_value_any));
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
          static_cast<Modules::Variable::state_t>(
              std::get<int64_t>(param_value_any)));
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
    case Event::Type::IO_LINK_REMOVE_EVENT:
    case Event::Type::IO_LINK_INSERT_EVENT:
      this->ioLinkUpdateCMD(cmd);
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
  if (this->task->task_finished) {
    return;
  }

  switch (event->getType()) {
    case Event::Type::RT_PERIOD_EVENT:
      this->setPeriod(event);
      break;
    case Event::Type::IO_BLOCK_QUERY_EVENT:
      this->blockInfoRequest(event);
      break;
    case Event::Type::IO_BLOCK_OUTPUTS_QUERY_EVENT:
      this->connectionsInfoRequest(event);
      break;
    case Event::Type::IO_ALL_CONNECTIONS_QUERY_EVENT:
      this->allConnectionsInfoRequest(event);
      break;
    case Event::Type::IO_LINK_INSERT_EVENT:
    case Event::Type::IO_LINK_REMOVE_EVENT:
      this->ioLinkChange(event);
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

void RT::System::NOOP(Event::Object* /*event*/)
{
  RT::System::CMD cmd(Event::Type::NOOP);
  RT::System::CMD* cmd_ptr = &cmd;
  this->eventFifo->write(&cmd_ptr, sizeof(RT::System::CMD*));
  cmd.wait();
}

void RT::System::insertDevice(Event::Object* event)
{
  auto* device = std::any_cast<RT::Device*>(event->getParam("device"));
  std::vector<RT::block_connection_t> connections_memory;
  connections_memory.reserve(10);
  if (device == nullptr) {
    ERROR_MSG("RT::System::insertDevice : invalid device pointer\n");
    return;
  }
  this->rt_connector->insertBlock(device, connections_memory);
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
  this->rt_connector->removeBlock(device);
  std::vector<RT::Device*> device_list = this->rt_connector->getDevices();
  RT::System::CMD cmd(event->getType());
  cmd.setRTParam("deviceList", &device_list);
  cmd.setRTParam("device", device);
  RT::System::CMD* cmd_ptr = &cmd;
  this->eventFifo->write(&cmd_ptr, sizeof(RT::System::CMD*));
  cmd.wait();
}

void RT::System::insertThread(Event::Object* event)
{
  auto* thread = std::any_cast<RT::Thread*>(event->getParam("thread"));
  std::vector<RT::block_connection_t> connections_memory;
  connections_memory.reserve(10);
  if (thread == nullptr) {
    ERROR_MSG("RT::System::removeDevice : invalid device pointer\n");
    return;
  }
  this->rt_connector->insertBlock(thread, connections_memory);
  std::vector<RT::Thread*> thread_list = this->rt_connector->getThreads();
  RT::System::CMD cmd(event->getType());
  cmd.setRTParam("threadList", &thread_list);
  RT::System::CMD* cmd_ptr = &cmd;
  this->eventFifo->write(&cmd_ptr, sizeof(RT::System::CMD*));
  cmd.wait();
}

// TODO: come back after figuring out conneciton problems
void RT::System::removeThread(Event::Object* event)
{
  auto* thread = std::any_cast<RT::Thread*>(event->getParam("thread"));
  if (thread == nullptr) {
    ERROR_MSG("RT::System::removeDevice : invalid device pointer\n");
    return;
  }
  // We have to make sure to deactivate thread before removing
  thread->setActive(/*act=*/false);
  this->rt_connector->removeBlock(thread);
  std::vector<RT::Thread*> thread_list = this->rt_connector->getThreads();
  RT::System::CMD cmd(event->getType());
  cmd.setRTParam("threadList", &thread_list);
  cmd.setRTParam("thread", thread);
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

void RT::System::ioLinkChange(Event::Object* event)
{
  RT::System::CMD cmd(event->getType());
  cmd.setRTParam(
      "connection",
      std::any_cast<RT::block_connection_t>(event->getParam("connection")));
  RT::System::CMD* cmd_ptr = &cmd;
  this->eventFifo->write(&cmd_ptr, sizeof(RT::System::CMD*));
  cmd.wait();
}

void RT::System::connectionsInfoRequest(Event::Object* event)
{
  auto* source = std::any_cast<IO::Block*>(event->getParam("block"));
  const std::vector<RT::block_connection_t> outputs =
      this->rt_connector->getOutputs(source);
  event->setParam("outputs", std::any(outputs));
}

void RT::System::allConnectionsInfoRequest(Event::Object* event)
{
  const std::vector<RT::block_connection_t> all_conn =
      this->rt_connector->getAllConnections();
  event->setParam("connections", std::any(all_conn));
}

void RT::System::blockInfoRequest(Event::Object* event)
{
  const std::vector<IO::Block*> blocks =
      this->rt_connector->getRegisteredBlocks();
  event->setParam("blockList", std::any(blocks));
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
  auto* component =
      std::any_cast<Modules::Component*>(event->getParam("paramModule"));
  cmd.setRTParam("paramModule", component);
  auto param_id = std::any_cast<size_t>(event->getParam("paramID"));
  cmd.setRTParam("paramID", param_id);
  auto param_type = std::any_cast<Modules::Variable::variable_t>(
      event->getParam("paramType"));
  cmd.setRTParam("paramType", param_type);
  std::any param_value_any = event->getParam("paramValue");
  switch (param_type) {
    case Modules::Variable::DOUBLE_PARAMETER:
      cmd.setRTParam("paramValue", std::any_cast<double>(param_value_any));
      break;
    case Modules::Variable::INT_PARAMETER:
      cmd.setRTParam("paramValue", std::any_cast<int64_t>(param_value_any));

      break;
    case Modules::Variable::UINT_PARAMETER:
      cmd.setRTParam("paramValue", std::any_cast<uint64_t>(param_value_any));
      break;
    case Modules::Variable::STATE:
      cmd.setRTParam(
          "paramValue",
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

RT::System::CMD::CMD(Event::Type et)
    : Event::Object(et)
{
}

RT::command_param_t RT::System::CMD::getRTParam(
    const std::string_view& param_name)
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
      system->rt_connector->propagateBlockConnections(iDevice);
    }

    for (auto* iThread : system->threads) {
      iThread->execute();
      system->rt_connector->propagateBlockConnections(iThread);
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
