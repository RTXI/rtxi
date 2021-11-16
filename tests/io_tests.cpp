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

