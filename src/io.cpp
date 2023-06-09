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

#include <algorithm>
#include <queue>
#include <string>

#include "io.hpp"

#include "debug.hpp"
// NOLINTBEGIN(cppcoreguidelines-pro-bounds-constant-array-index)
IO::Block::Block(std::string blockname,
                 const std::vector<IO::channel_t>& channels,
                 bool isdependent)
    : id(0)
    , name(std::move(blockname))
    , isInputDependent(isdependent)
{
  port_t port = {};
  for (const auto& channel : channels) {
    port.channel_info = channel;
    port.values = std::vector<double>(channel.data_size, 0.0);
    port.buff_values = std::vector<double>(channel.data_size, 0.0);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
    ports[channel.flags].push_back(port);
    port = {};
  }
}

size_t IO::Block::getCount(IO::flags_t type) const
{
  // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
  return this->ports[type].size();
}

std::string IO::Block::getChannelName(IO::flags_t type, size_t index) const
{
  // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
  return this->ports[type][index].channel_info.name;
}

std::string IO::Block::getChannelDescription(IO::flags_t type,
                                             size_t index) const
{
  // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
  return this->ports[type][index].channel_info.description;
}

size_t IO::Block::getChannelSize(IO::flags_t type, size_t channel)
{
  // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
  return this->ports[type][channel].values.size();
}

void IO::Block::writeinput(size_t index, const std::vector<double>& data)
{
  //std::copy(
  //    data.begin(), data.end(), this->ports[IO::INPUT][index].values.begin());
  size_t value_index = 0;
  while(value_index < data.size()){
    this->ports[IO::INPUT][index].buff_values[value_index] += data[value_index];
    value_index++;
  }
}

const std::vector<double>& IO::Block::readinput(size_t index)
{
  // We must reset input values to zero so that the next cycle doesn't use these values
  std::copy(this->ports[IO::INPUT][index].buff_values.begin(),
            this->ports[IO::INPUT][index].buff_values.end(),
            this->ports[IO::INPUT][index].values.begin());
  std::fill(this->ports[IO::INPUT][index].buff_values.begin(),
            this->ports[IO::INPUT][index].buff_values.end(),
            0.0);
  return this->ports[IO::INPUT][index].values;
}

void IO::Block::writeoutput(size_t index, const std::vector<double>& data)
{
  std::copy(
      data.begin(), data.end(), this->ports[IO::OUTPUT][index].values.begin());
}

const std::vector<double>& IO::Block::readPort(IO::flags_t direction, size_t index)
{
  return this->ports[direction][index].values;
}
// NOLINTEND(cppcoreguidelines-pro-bounds-constant-array-index)
