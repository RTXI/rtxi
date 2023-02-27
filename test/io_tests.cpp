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
#include <vector>

#include "io_tests.hpp"

std::vector<IO::channel_t> generateDefaultChannelList()
{
  std::string defaultBlockName;
  std::string defaultInputChannelName = "CHANNEL INPUT";
  std::string defaultInputChannelDescription =
      "DEFAULT INPUT CHANNEL DESCRIPTION";
  std::string defaultOutputChannelName = "CHANNEL OUTPUT";
  std::string defaultOutputChannelDescription =
      "DEFAULT OUTPUT CHANNEL DESCRIPTION";
  std::vector<IO::channel_t> defaultChannelList;

  // Generates a default block with single input and output channel
  defaultBlockName = "DEFAULT:BLOCK:NAME";
  IO::channel_t defaultInputChannel = {};
  defaultInputChannel.name = defaultInputChannelName;
  defaultInputChannel.description = defaultInputChannelDescription;
  defaultInputChannel.flags = IO::INPUT;
  defaultInputChannel.data_size = 1;
  IO::channel_t defaultOutputChannel = {};
  defaultOutputChannel.name = defaultOutputChannelName;
  defaultOutputChannel.description = defaultOutputChannelDescription;
  defaultOutputChannel.flags = IO::OUTPUT;
  defaultOutputChannel.data_size = 1;
  defaultChannelList.push_back(defaultInputChannel);
  defaultChannelList.push_back(defaultOutputChannel);

  return defaultChannelList;
}

TEST_F(IOBlockTest, getName)
{
  IO::Block block(this->defaultBlockName, this->defaultChannelList, true);
  ASSERT_EQ(block.getName(), defaultBlockName);
  ASSERT_EQ(block.getChannelName(IO::INPUT, 0), defaultInputChannelName);
  ASSERT_EQ(block.getChannelName(IO::OUTPUT, 0), defaultOutputChannelName);
}

TEST_F(IOBlockTest, getCount)
{
  IO::Block block(this->defaultBlockName, this->defaultChannelList, true);
  ASSERT_EQ(block.getCount(IO::INPUT), 1);
  ASSERT_EQ(block.getCount(IO::OUTPUT), 1);
}

TEST_F(IOBlockTest, getDescription)
{
  IO::Block block(this->defaultBlockName, this->defaultChannelList, true);
  ASSERT_EQ(block.getChannelDescription(IO::INPUT, 0),
            defaultInputChannelDescription);
  ASSERT_EQ(block.getChannelDescription(IO::OUTPUT, 0),
            defaultOutputChannelDescription);
}

TEST_F(IOBlockTest, readoutput)
{
  IO::Block block(this->defaultBlockName, this->defaultChannelList, true);
  std::vector<double> defaultval = {0.0};
  EXPECT_DOUBLE_EQ(defaultval[0], block.readoutput(0)[0]);
}

TEST_F(IOBlockTest, writeinput)
{
  std::vector<double> values = {1.0};
  class testBlock : public IO::Block
  {
  public:
    testBlock(std::string n, const std::vector<IO::channel_t>& c)
        : IO::Block(n, c, true)
    {
    }
    void echo() { this->writeoutput(0, this->readinput(0)); }
  };
  testBlock tempblock("TEST:BLOCK:NAME", this->defaultChannelList);
  tempblock.writeinput(0, values);
  tempblock.echo();
  EXPECT_DOUBLE_EQ(values[0], tempblock.readoutput(0)[0]);
}
