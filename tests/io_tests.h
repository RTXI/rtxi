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
    IO::channel_t defaultChannel_t;
    std::string defaultChannelName;
    std::string defaultChannelDescription;
    IO::flags_t defaultChannelFlags;

protected:
    IOBlockTest() 
    {
        defaultBlockName = "DEFAULT:BLOCK:NAME";
        defaultChannelName = "DEFAULT:CHANNEL:NAME";
        defaultChannelDescription = "DEFAULT:CHANNEL:DESCRIPTION";
        defaultChannelFlags = IO::INPUT;
        defaultChannel_t = { 
            defaultChannelName,
            defaultChannelDescription,
            defaultChannelFlags,
        };
        block = new IO::Block(defaultBlockName, &defaultChannel_t, (size_t) 1);
    }
    ~IOBlockTest() { }

    IO::Block *block;
};

#endif
