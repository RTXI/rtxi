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

#include <algorithm>
#include <sstream>
#include <string>
#include <queue>

#include "io.hpp"

#include "debug.hpp"
// NOLINTBEGIN(cppcoreguidelines-pro-bounds-constant-array-index)
IO::Block::Block(std::string n, const std::vector<IO::channel_t>& channels, bool isdependent)
    : name(std::move(n)) , isInputDependent(isdependent)
{
  port_t port = {};
  for (const auto& channel : channels) {
    port.channel_info = channel;
    port.values = std::vector<double>(channel.data_size, 0.0);
    ports[channel.flags].push_back(port);
    port = {};
  }
}

size_t IO::Block::getCount(IO::flags_t type) const
{
  return this->ports[type].size();
}

std::string IO::Block::getChannelName(IO::flags_t type, size_t index) const
{
  return this->ports[type][index].channel_info.name;
}

std::string IO::Block::getChannelDescription(IO::flags_t type, size_t index) const
{
  return this->ports[type][index].channel_info.description;
}

void IO::Block::writeinput(size_t index, const std::vector<double>& data)
{
  std::copy(
      data.begin(), data.end(), this->ports[IO::INPUT][index].values.begin());
}

const std::vector<double>& IO::Block::readinput(size_t index)
{
  return this->ports[IO::INPUT][index].values;
}

void IO::Block::writeoutput(size_t index, const std::vector<double>& data)
{
  std::copy(
      data.begin(), data.end(), this->ports[IO::OUTPUT][index].values.begin());
}

const std::vector<double>& IO::Block::readoutput(size_t index)
{
  return this->ports[IO::OUTPUT][index].values;
}

int IO::Connector::connect(IO::Block* src,
                            size_t out,
                            IO::Block* dest,
                            size_t in)
{
  // First we must make sure that we aren't going to create a cycle
  if(dest == src){ return -1; } // can't be connected to itself
  if(src->dependent()) // we check for cycles only if src has dependencies (RT::Thread)
  {
    std::vector<IO::Block*> connected_blocks;
    std::queue<IO::Block*> processing;
    processing.push(dest);
    std::vector<IO::Block*>::iterator loc;
    while(!processing.empty())
    {
      loc = std::find(connected_blocks.begin(), connected_blocks.end(), processing.front());
      if(loc == connected_blocks.end()) { 
        connected_blocks.push_back(processing.front());
        for(auto connection : this->registry[processing.front()]){
          processing.push(connection.dest);
        }
      }
      loc = std::find(connected_blocks.begin(), connected_blocks.end(), src);
      if(loc != connected_blocks.end()){ return -1; }
      processing.pop();
    }
  }

  // now create connection object and save it
  IO::outputs_info out_con = {};
  out_con.dest = dest;
  out_con.src_port = out;
  out_con.dest_port = in;
  if (!(this->registry.contains(src))) {
    this->registry[src] = std::vector<IO::outputs_info>();
  }
  this->registry[src].push_back(out_con);
  return 0;
}

void IO::Connector::disconnect(IO::Block* src,
                               size_t out,
                               IO::Block* dest,
                               size_t in)
{
  if(!(this->registry.contains(src))){ return; }
  auto loc = std::find_if(
    this->registry[src].begin(),
    this->registry[src].end(),
    [&](IO::outputs_info out_con){ 
      return out_con.dest == dest 
        && out_con.src_port == out 
        && out_con.dest_port == in;
      }
  );
  if(loc != this->registry[src].end()){
    this->registry[src].erase(loc);
  }
}


bool IO::Connector::connected(IO::Block* src,
                              size_t out,
                              IO::Block* dest,
                              size_t in)
{
  if(!(registry.contains(src))){ return false;}
  auto loc = std::find_if(
    this->registry[src].begin(),
    this->registry[src].end(),
    [&](outputs_info out_con){ return out_con.dest == dest;}
  );
  if(loc == this->registry[src].end()) { return false; }
  return loc->src_port == out && loc->dest_port == in;
}

void IO::Connector::insertBlock(IO::Block* block)
{
  if (block == nullptr) {
    ERROR_MSG("IO::Connector::insertBlock : nullptr block\n");
    return;
  }

  this->registry[block] = std::vector<IO::outputs_info>();
}

void IO::Connector::removeBlock(IO::Block* block)
{
  if (block == nullptr) {
    ERROR_MSG("IO::Connector::removeBlock : nullptr block\n");
    return;
  }
  // first we have to disconnect all blocks that are connected to this
  for(const auto& block_connections : registry){
    for(const auto connection : block_connections.second){
      if(connection.dest == block){
        this->disconnect(block_connections.first, 
                         connection.src_port, 
                         connection.dest, 
                         connection.dest_port);
      }
    }
  }
  this->registry.erase(block);
}

std::vector<IO::Block*> IO::Connector::topological_sort()
{
  std::queue<IO::Block*> processing_q;
  std::vector<IO::Block*> sorted_blocks;
  std::unordered_map<IO::Block*, int> sources_per_block;

  // initialize counts
  for(const auto& block : this->registry){
    sources_per_block[block.first] = 0;
  }

  // Calculate number of sources per block
  for(const auto& outputs : this->registry){
    if(!(outputs.first->dependent())) { continue; } // topology sort skips devices
    for(auto destination_con : outputs.second){
      sources_per_block[destination_con.dest] += 1;
    }
  }

  // Initialize queue for processing nodes in graph
  for(auto block_count : sources_per_block){
    if(block_count.second == 0 && block_count.first->dependent()){ 
      processing_q.push(block_count.first); 
    }
  }

  // Process the graph nodes. Don't add independent blocks to processing queue
  while(!processing_q.empty()){
    sorted_blocks.push_back(processing_q.front());
    processing_q.pop();
    for(auto connections : registry[sorted_blocks.back()]){
      if(!(connections.dest->dependent())) { continue; }
      sources_per_block[connections.dest] -= 1;
      if(sources_per_block[connections.dest] == 0){
        processing_q.push(connections.dest);
      }
    }
  }
  
  return sorted_blocks;
}

std::vector<IO::Block*> IO::Connector::getDevices()
{
  std::vector<IO::Block*> devices;
  for(const auto& block_info : this->registry)
  {
    if(!block_info.first->dependent()) { devices.push_back(block_info.first); }
  }
  return devices;
}

std::vector<IO::Block*> IO::Connector::getThreads()
{
  return this->topological_sort();
}

std::vector<IO::outputs_info> IO::Connector::getOutputs(IO::Block* src)
{
  auto result = std::vector<IO::outputs_info>();
  if(this->registry.contains(src)) { result = this->registry[src]; }
  return result;
}
// NOLINTEND(cppcoreguidelines-pro-bounds-constant-array-index)