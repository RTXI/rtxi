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

std::string IO::Block::getChannelDescription(IO::flags_t type, size_t index) const
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
// NOLINTEND(cppcoreguidelines-pro-bounds-constant-array-index)
