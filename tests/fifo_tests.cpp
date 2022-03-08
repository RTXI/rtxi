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

#include <fifo_tests.h>
#include <stdio.h>

TEST_F(FifoTest, ReadAndWrite)
{
    fifo = new Fifo((size_t) 11); 
    char inbuff[10];
    char outbuff[10];
    for(int i=0; i<9; i++){
        inbuff[i] = (char) i+97;
    }
    inbuff[9] = '\0';
    fifo->write(inbuff, (size_t) 10);
    fifo->read(outbuff, (size_t) 10);
    EXPECT_STREQ(inbuff, outbuff);
    delete fifo;
}


