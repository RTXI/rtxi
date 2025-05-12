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

#include <string>

#include "io.hpp"

// NOLINTBEGIN(cppcoreguidelines-pro-bounds-constant-array-index)
IO::Block::Block(std::string blockname,
                 const std::vector<IO::channel_t>& channels,
                 bool isdependent)
    : name(std::move(blockname))
    , isInputDependent(isdependent)
{
  port_t port = {};
  for (const auto& channel : channels) {
    port.channel_info = channel;
    port.value = 0.0;
    port.buff_value = 0.0;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
    ports.at(channel.flags).push_back(port);
    port = {};
  }
}

size_t IO::Block::getCount(IO::flags_t type) const
{
  // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
  return this->ports.at(type).size();
}

std::string IO::Block::getChannelName(IO::flags_t type, size_t index) const
{
  // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
  return this->ports.at(type).at(index).channel_info.name;
}

std::string IO::Block::getChannelDescription(IO::flags_t type,
                                             size_t index) const
{
  // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
  return this->ports.at(type).at(index).channel_info.description;
}

void IO::Block::writeinput(size_t index, const double& data)
{
  this->ports.at(IO::INPUT).at(index).buff_value += data;
}

double& IO::Block::readinput(size_t index)
{
  // We must reset input values to zero so that the next cycle doesn't use these
  // values
  this->ports.at(IO::INPUT).at(index).value =
      this->ports.at(IO::INPUT).at(index).buff_value;
  this->ports.at(IO::INPUT).at(index).buff_value = 0.0;
  return this->ports.at(IO::INPUT).at(index).value;
}

void IO::Block::writeoutput(size_t index, const double& data)
{
  this->ports.at(IO::OUTPUT).at(index).value = data;
}

const double& IO::Block::readPort(IO::flags_t direction, size_t index)
{
  return this->ports.at(direction).at(index).value;
}
// NOLINTEND(cppcoreguidelines-pro-bounds-constant-array-index)
