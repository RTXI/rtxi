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

#include "io.hpp"

#include "debug.hpp"

IO::Block::Block(std::string n, std::vector<IO::channel_t> channels)
    : name(n)
{
  port_t port = {};
  for (auto channel : channels) {
    port.channel_info = channel;
    port.values = std::vector<double>(channel.data_size, 0.0);
    ports[channel.flags].push_back(port);
    port = {};
  }
}

IO::Block::~Block() {}

size_t IO::Block::getCount(IO::flags_t type) const
{
  return this->ports[type].size();
}

std::string IO::Block::getChannelName(IO::flags_t type, size_t n) const
{
  return this->ports[type][n].channel_info.name;
}

std::string IO::Block::getChannelDescription(IO::flags_t type, size_t n) const
{
  return this->ports[type][n].channel_info.description;
}

std::vector<double> IO::Block::getChannelValue(IO::flags_t type, size_t n) const
{
  return this->ports[type][n].values;
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
  auto compare_con = [&](connection_t* con)
  {
    return con->src == src && con->src_port == out && con->dest == dest
        && con->dest_port == in;
  };

  auto loc = std::find_if(
      this->connections.begin(), this->connections.end(), compare_con);
  if (loc == this->connections.end()) {
    connection_t con = {};
    con.src = src;
    con.src_port = out;
    con.dest = dest;
    con.dest_port = in;
    this->connections.push_back(con);
  }
}

void IO::Connector::disconnect(IO::Block* src,
                               size_t out,
                               IO::Block* dest,
                               size_t in)
{
  auto compare_con = [&](connection_t* con)
  {
    return con->src == src && con->src_port == out && con->dest == dest
        && con->dest_port == in;
  };

  auto loc = std::find_if(
      this->connections.begin(), this->connections.end(), compare_con);
  if (loc != this->connections.end()) {
    this->connections.erase(loc);
  }
}

bool IO::Connector::connected(IO::Block* src,
                              size_t out,
                              IO::Block* dest,
                              size_t in)
{
  auto compare_con = [&](connection_t* con)
  {
    return con->src == src && con->src_port == out && con->dest == dest
        && con->dest_port == in;
  };
  auto loc = std::find_if(
      this->connections.begin(), this->connections.end(), compare_con);
  if (loc != this->connections.end())
    return true;
  return false;
}

void IO::Connector::insertBlock(IO::Block* block)
{
  if (!block) {
    ERROR_MSG("IO::Connector::insertBlock : invalid block\n");
    return;
  }

  if (std::find(this->registry.begin(), this->registry.end(), block)
      != this->registry.end())
  {
    return;
  }

  registry.push_back(block);
}

void IO::Connector::removeBlock(IO::Block* block)
{
  if (!block) {
    ERROR_MSG("IO::Connector::insertBlock : invalid block\n");
    return;
  }
  auto position =
      std::find(this->registry.begin(), this->registry.end(), block);
  this->registry.erase(position);
}

bool IO::Connector::acyclical()
{

}

std::vector<IO::connection_t> IO::Connector::topological_sort()
{

}