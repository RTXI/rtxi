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

#include <iostream>
#include <algorithm>
#include <numeric>
#include <random>
#include <vector>
#include <memory>

#include "io_tests.hpp"

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
    void echo()
    {
      this->writeoutput(0, this->readinput(0));
    }
  };
  testBlock tempblock("TEST:BLOCK:NAME", this->defaultChannelList);
  tempblock.writeinput(0, values);
  tempblock.echo();
  EXPECT_DOUBLE_EQ(values[0], tempblock.readoutput(0)[0]);
}

TEST_F(IOConnectorTest, connections)
{
  // Have to build example blocks to test connector
  IO::Block block1("BLOCK1", this->defaultChannelList, true);
  IO::Block block2("BLOCK2", this->defaultChannelList, true);

  // connect and disconnect between two blocks
  EXPECT_FALSE(this->connector.connected(&block1, 0, &block2, 0));
  EXPECT_FALSE(this->connector.connected(&block2, 0, &block1, 0));
  int result = 0;
  result = this->connector.connect(&block1, 0, &block2, 0);
  ASSERT_EQ(result, 0);
  result = this->connector.connect(&block2, 0, &block1, 0);
  ASSERT_EQ(result, -1);
  EXPECT_TRUE(this->connector.connected(&block1, 0, &block2, 0));
  EXPECT_FALSE(this->connector.connected(&block2, 0, &block1, 0));
  this->connector.disconnect(&block1, 0, &block2, 0);
  this->connector.disconnect(&block2, 0, &block1, 0);
  EXPECT_FALSE(this->connector.connected(&block1, 0, &block2, 0));
  EXPECT_FALSE(this->connector.connected(&block2, 0, &block1, 0));
}

TEST_F(IOConnectorTest, getOutputs)
{
  IO::Block outputblock(this->defaultBlockName, this->defaultChannelList, true);
  std::vector<std::unique_ptr<IO::Block>> inputblocks;
  for(int i=0; i<100; i++){
    inputblocks.push_back(std::make_unique<IO::Block>("randblock", this->defaultChannelList, true));
  }
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> distribution(0,1);
  std::vector<int> randvals(100);
  for(auto& val : randvals){ val = distribution(gen); }
  for(int i=0; i<100; i++){
    if(randvals[i] == 1){
      this->connector.connect(&outputblock, 0, inputblocks[i].get(), 0);
    }
  }
  std::vector<IO::outputs_info> output_connections = this->connector.getOutputs(&outputblock);
  int num_of_connections = std::accumulate(randvals.begin(), randvals.end(), 0);
  EXPECT_EQ(output_connections.size(), num_of_connections);
  for(auto con : output_connections){
    ASSERT_TRUE(this->connector.connected(&outputblock, 0, con.dest, 0));
  }
}

TEST_F(IOConnectorTest, getBlocks)
{
  std::vector<std::unique_ptr<IO::Block>> blocks(50);
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> distribution(0,49);
  for(int i=0; i<50; i++){
    blocks[i] = std::make_unique<IO::Block>("randblock", this->defaultChannelList, distribution(gen) >= 25);
    this->connector.insertBlock(blocks[i].get());
  }
  for(auto iter=blocks.begin(); iter != blocks.end(); iter++){
    this->connector.connect(iter->get(), 0, blocks[distribution(gen)].get(), 0);
    this->connector.connect(iter->get(), 0, blocks[distribution(gen)].get(), 0);
    this->connector.connect(iter->get(), 0, blocks[distribution(gen)].get(), 0);
  }
  std::vector<IO::Block*> threads = this->connector.getThreads();

  // first verify that they are in topological order
  for(auto thread_iter=threads.begin(); thread_iter != threads.end(); thread_iter++){
    for(auto outputs : this->connector.getOutputs(*thread_iter)){
      auto loc = std::find(threads.begin(), thread_iter, outputs.dest);
      ASSERT_EQ(loc, thread_iter);
    }
  }
  std::vector<IO::Block*> devices = this->connector.getDevices();
  ASSERT_EQ(devices.size()+threads.size(), 50);
}


