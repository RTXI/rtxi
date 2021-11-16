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

#ifndef IO_TESTS_H
#define IO_TESTS_H

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <io.h>

class IOBlockTest : public ::testing::Test
{
public:
    std::string defaultBlockName;
    std::string defaultInputChannelName = "CHANNEL INPUT";
    std::string defaultInputChannelDescription = "DEFAULT INPUT CHANNEL DESCRIPTION";
    std::string defaultOutputChannelName = "CHANNEL OUTPUT";
    std::string defaultOutputChannelDescription = "DEFAULT OUTPUT CHANNEL DESCRIPTION";
    IO::channel_t *defaultChannelList;

protected:
    IOBlockTest() 
    {
        // Generates a default block with single input and output channel
        defaultBlockName = "DEFAULT:BLOCK:NAME";
        IO::channel_t defaultInputChannel_t = { 
            defaultInputChannelName,
            defaultInputChannelDescription,
            IO::INPUT
        };
        IO::channel_t defaultOutputChannel_t = {
            defaultOutputChannelName,
            defaultOutputChannelDescription,
            IO::OUTPUT
        };
        defaultChannelList = new IO::channel_t[2];
        defaultChannelList[0] = defaultInputChannel_t;
        defaultChannelList[1] = defaultOutputChannel_t;
        block = new IO::Block(defaultBlockName, defaultChannelList, (size_t) 2);
    }
    ~IOBlockTest() { }

    IO::Block *block;
};

#endif
