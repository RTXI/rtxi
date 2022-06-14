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
IO::Block::Block(std::string n, const std::vector<IO::channel_t>& channels)
    : name(std::move(n))
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

void IO::Connector::connect(IO::Block* src,
                            size_t out,
                            IO::Block* dest,
                            size_t in)
{
  outputs_con out_con = {};
  out_con.destblock = dest;
  out_con.srcport = out;
  out_con.destport = in;
  if (!(this->registry.contains(src))) {
    this->registry[src] = std::vector<outputs_con>();
  }
  this->registry[src].push_back(out_con);
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
    [&](outputs_con out_con){ 
      return out_con.destblock == dest 
        && out_con.srcport == out 
        && out_con.destport == in;
      }
  );
  this->registry[src].erase(loc);
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
    [&](outputs_con out_con){ return out_con.destblock == dest;}
  );
  if(loc == this->registry[src].end()) { return false; }
  return loc->srcport == out && loc->destport == in;
}

void IO::Connector::insertBlock(IO::Block* block)
{
  if (block == nullptr) {
    ERROR_MSG("IO::Connector::insertBlock : nullptr block\n");
    return;
  }
  this->registry[block] = std::vector<outputs_con>();
}

void IO::Connector::removeBlock(IO::Block* block)
{
  if (block == nullptr) {
    ERROR_MSG("IO::Connector::removeBlock : nullptr block\n");
    return;
  }
  this->registry.erase(block);
}

std::vector<IO::Block*> IO::Connector::topological_sort()
{
  std::queue<IO::Block*> processing_q;
  std::vector<IO::Block*> sorted_blocks;
  std::unordered_map<IO::Block*, int> sources_per_block;

  // Calculate number of sources per block
  for(const auto& outputs : registry){
    for(auto destination_con : outputs.second){
      if(sources_per_block.contains(destination_con.destblock)) {
        sources_per_block[destination_con.destblock] += 1;
      } else {
        sources_per_block[destination_con.destblock] = 1;
      }
    }
  }

  // Initialize queue for processing nodes in graph
  for(auto block_count : sources_per_block){
    if(block_count.second == 0){ processing_q.push(block_count.first); }
  }

  // Process the graph nodes
  while(!processing_q.empty()){
    sorted_blocks.push_back(processing_q.front());
    processing_q.pop();
    for(auto connection_info : registry[sorted_blocks.back()]){
      sources_per_block[connection_info.destblock] -= 1;
      if(sources_per_block[connection_info.destblock] == 0){
        processing_q.push(connection_info.destblock);
      }
    }
  }
  
  return sorted_blocks;
}
// NOLINTEND(cppcoreguidelines-pro-bounds-constant-array-index)