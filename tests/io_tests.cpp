/*
 	 The Real-Time eXperiment Interface (RTXI)
	 Copyright (C) 2011 Georgia Institute of Technology, University of Utah, Weill Cornell Medical College

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

#include <io.h>
#include <iostream>
#include <io_tests.h>

TEST_F(IOBlockTest, getName)
{
    // TODO: rename functions to differentiate between getting block name and channel name
    ASSERT_EQ(block->getName(), defaultBlockName);
    ASSERT_EQ(block->getName(IO::INPUT, (size_t) 0), defaultInputChannelName);
    ASSERT_EQ(block->getName(IO::OUTPUT, (size_t) 0), defaultOutputChannelName);
}

TEST_F(IOBlockTest, getCount)
{
    ASSERT_EQ(block->getCount(IO::INPUT), (size_t) 1);
    ASSERT_EQ(block->getCount(IO::OUTPUT), (size_t) 1);
}

TEST_F(IOBlockTest, getDescription)
{
    ASSERT_EQ(block->getDescription(IO::INPUT, (size_t) 0), defaultInputChannelDescription);
    ASSERT_EQ(block->getDescription(IO::OUTPUT, (size_t) 0), defaultOutputChannelDescription);
}

TEST_F(IOBlockTest, getValue)
{
    double defaultval = 0.0;
    EXPECT_DOUBLE_EQ(defaultval, block->getValue(IO::INPUT, (size_t) 1));
    EXPECT_DOUBLE_EQ(defaultval, block->getValue(IO::OUTPUT, (size_t) 1));
}

TEST_F(IOBlockTest, input)
{
    double defaultval = 0.0;
    for(size_t i = 0; i < 2; ++i)
    {
        EXPECT_DOUBLE_EQ(defaultval, block->input(i));
    }

    // Have to build example blocks to test connector
    IO::channel_t outputchannel = {
        "OUTPUT CHANNEL",
        "OUTPUT CHANNEL DESCRIPTION",
        IO::OUTPUT
    };
    IO::channel_t inputchannel = {
        "INPUT CHANNEL",
        "INPUT CHANNEL DESCRIPTION",
        IO::INPUT
    };

    IO::channel_t *block1channels = new IO::channel_t[2];
    block1channels[0] = outputchannel;
    block1channels[1] = inputchannel;
    IO::channel_t *block2channels = new IO::channel_t[2];
    block2channels[0] = outputchannel;
    block2channels[1] = inputchannel;
    MockIOBlock *block1 = new MockIOBlock("block1", block1channels, (size_t) 2);
    MockIOBlock *block2 = new MockIOBlock("block2", block2channels, (size_t) 2);

    // Now connect the test blocks to default block input channel
    IO::Connector::getInstance()->connect(block1, (size_t) 0, block, (size_t) 0);
    IO::Connector::getInstance()->connect(block2, (size_t) 0, block, (size_t) 0);
    block1->changeOutput(1.0);
    EXPECT_DOUBLE_EQ(1.0, block->input(0));
    block2->changeOutput(1.0);
    EXPECT_DOUBLE_EQ(2.0, block->input(0));

    // please cleanup after yourself
    delete block1;
    delete block2;
}

TEST_F(IOBlockTest, output)
{
    double defaultval = 0.0;
    // TODO: Maybe reduce output function to single definition instead of two
    const IO::Block *const_block = block;
    for(size_t i = 0; i < 2; ++i)
    {
        EXPECT_DOUBLE_EQ(defaultval, const_block->output(i));
    }
}

TEST_F(IOConnectorTest, getInstance)
{
    connector = IO::Connector::getInstance();
    EXPECT_EQ(connector, IO::Connector::getInstance());
    EXPECT_EQ(connector, connector->getInstance());
}

// checking connections is very involved so could not separate into 
// individual functin tests. Here we test connect, disconnect, and connected
TEST_F(IOConnectorTest, connections)
{
    // Have to build example blocks to test connector
    IO::channel_t outputchannel = {
        "OUTPUT CHANNEL",
        "OUTPUT CHANNEL DESCRIPTION",
        IO::OUTPUT
    };
    IO::channel_t inputchannel = {
        "INPUT CHANNEL",
        "INPUT CHANNEL DESCRIPTION",
        IO::INPUT
    };
    IO::channel_t *block1channels = new IO::channel_t[2];
    block1channels[0] = outputchannel;
    block1channels[1] = inputchannel;
    IO::channel_t *block2channels = new IO::channel_t[2];
    block2channels[0] = outputchannel;
    block2channels[1] = inputchannel;
    MockIOBlock *block1 = new MockIOBlock("block1", block1channels, (size_t) 2);
    MockIOBlock *block2 = new MockIOBlock("block2", block2channels, (size_t) 2);

    // NOTE: It is not possible to mock static functions, therefore we must use
    // other means to figure out whether two blocks are connected

    // connect and disconnect between two blocks
    EXPECT_FALSE(connector->connected(block1, (size_t) 0, block2, (size_t) 0)); 
    EXPECT_FALSE(connector->connected(block2, (size_t) 0, block1, (size_t) 0)); 
    connector->connect(block1, (size_t) 0, block2, (size_t) 0);
    connector->connect(block2, (size_t) 0, block1, (size_t) 0);
    EXPECT_TRUE(connector->connected(block1, (size_t) 0, block2, (size_t) 0));
    EXPECT_TRUE(connector->connected(block2, (size_t) 0, block1, (size_t) 0)); 
    connector->disconnect(block1, (size_t) 0, block2, (size_t) 0);
    connector->disconnect(block2, (size_t) 0, block1, (size_t) 0);
    EXPECT_FALSE(connector->connected(block1, (size_t) 0, block2, (size_t) 0)); 
    EXPECT_FALSE(connector->connected(block2, (size_t) 0, block1, (size_t) 0)); 
    delete block1;
    delete block2;
}

// The cases where foreachBlock function is used are too unique (only used a handful of times)
// TODO: Create a test case for IO::Connector::foreachBlock
TEST_F(IOConnectorTest, foreachBlock)
{
}

// the cases where foreachConnection function is used are too unique (only used a handful of times)
// TODO: create a test case for IO::Connector::foreachConnection
TEST_F(IOConnectorTest, foreachConnection)
{

}


