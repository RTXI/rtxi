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
#include <iostream>
#include <memory>
#include <numeric>
#include <utility>
#include <vector>

#include "io_tests.hpp"

std::vector<IO::channel_t> generateDefaultChannelList()
{
  std::string defaultBlockName;
  const std::string defaultInputChannelName = "CHANNEL INPUT";
  const std::string defaultInputChannelDescription =
      "DEFAULT INPUT CHANNEL DESCRIPTION";
  const std::string defaultOutputChannelName = "CHANNEL OUTPUT";
  const std::string defaultOutputChannelDescription =
      "DEFAULT OUTPUT CHANNEL DESCRIPTION";
  std::vector<IO::channel_t> defaultChannelList;

  // Generates a default block with single input and output channel
  defaultBlockName = "DEFAULT:BLOCK:NAME";
  IO::channel_t defaultInputChannel = {};
  defaultInputChannel.name = defaultInputChannelName;
  defaultInputChannel.description = defaultInputChannelDescription;
  defaultInputChannel.flags = IO::INPUT;
  IO::channel_t defaultOutputChannel = {};
  defaultOutputChannel.name = defaultOutputChannelName;
  defaultOutputChannel.description = defaultOutputChannelDescription;
  defaultOutputChannel.flags = IO::OUTPUT;
  defaultChannelList.push_back(defaultInputChannel);
  defaultChannelList.push_back(defaultOutputChannel);

  return defaultChannelList;
}

TEST_F(IOBlockTest, getName)
{
  const IO::Block block(
      this->defaultBlockName, this->defaultChannelList, /*isdependent=*/true);
  ASSERT_EQ(block.getName(), defaultBlockName);
  ASSERT_EQ(block.getChannelName(IO::INPUT, 0), defaultInputChannelName);
  ASSERT_EQ(block.getChannelName(IO::OUTPUT, 0), defaultOutputChannelName);
}

TEST_F(IOBlockTest, getCount)
{
  const IO::Block block(
      this->defaultBlockName, this->defaultChannelList, /*isdependent=*/true);
  ASSERT_EQ(block.getCount(IO::INPUT), 1);
  ASSERT_EQ(block.getCount(IO::OUTPUT), 1);
}

TEST_F(IOBlockTest, getDescription)
{
  const IO::Block block(
      this->defaultBlockName, this->defaultChannelList, /*isdependent=*/true);
  ASSERT_EQ(block.getChannelDescription(IO::INPUT, 0),
            defaultInputChannelDescription);
  ASSERT_EQ(block.getChannelDescription(IO::OUTPUT, 0),
            defaultOutputChannelDescription);
}

TEST_F(IOBlockTest, readPort)
{
  IO::Block block(
      this->defaultBlockName, this->defaultChannelList, /*isdependent=*/true);
  const double defaultval = 0.0;
  EXPECT_DOUBLE_EQ(defaultval, block.readPort(IO::OUTPUT, 0));
}

TEST_F(IOBlockTest, writeinput)
{
  const double values = 1.0;
  class testBlock : public IO::Block
  {
  public:
    testBlock(std::string n, const std::vector<IO::channel_t>& c)
        : IO::Block(std::move(n), c, /*isdependent=*/true)
    {
    }
    void echo() { this->writeoutput(0, this->readinput(0)); }
  };
  testBlock tempblock("TEST:BLOCK:NAME", this->defaultChannelList);
  tempblock.writeinput(0, values);
  tempblock.echo();
  EXPECT_DOUBLE_EQ(values, tempblock.readPort(IO::OUTPUT, 0));
  tempblock.writeinput(0, values);
  tempblock.writeinput(0, values);
  tempblock.echo();
  EXPECT_DOUBLE_EQ(values + values, tempblock.readPort(IO::OUTPUT, 0));
}
