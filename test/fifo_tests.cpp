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
#include <thread>

TEST_F(FifoTest, ReadAndWrite)
{
    fifo = new Fifo((size_t) 100); 
    char inbuff[22];
    char outbuff[22];
    size_t written_bytes;
    size_t read_bytes;
    for(int i=0; i<9; i++){
        inbuff[i] = (char) i+97;
    }
    inbuff[9] = '\0';

    // There should be zero bits read if the fifo is empty
    EXPECT_EQ((size_t) 0, fifo->read(outbuff, (size_t) 10, false));

    // check that read and write work properly

    written_bytes = fifo->write(inbuff, (size_t) 21);
    read_bytes = fifo->read(outbuff, (size_t) 21);
    EXPECT_STREQ(inbuff, outbuff);
    EXPECT_EQ(written_bytes, read_bytes);

    // The fifo should be empty after reading
    EXPECT_EQ((size_t) 0, fifo->read(outbuff, (size_t) 21, false));

    // should be able to write multiple times
    for(int i = 0; i < 10; i++){
        written_bytes = fifo->write(inbuff, (size_t) 11);
        read_bytes = fifo->read(outbuff, (size_t) 11);
        EXPECT_STREQ(inbuff, outbuff);
        EXPECT_EQ(written_bytes, read_bytes);
    }

    delete fifo;
}

TEST_F(FifoTest, Failures)
{
    fifo = new Fifo((size_t) 2);
    char buff[8] = "message";

    // Test whether FIFO fails when overwritting to it
    EXPECT_EQ(fifo->write(buff, (size_t) 8), (size_t) 0);
}

TEST_F(FifoTest, Threaded)
{
    char message[8] = "message";
    char output[8]; 
    size_t size = 8;
    fifo = new Fifo((size_t) 10);
    std::thread sender(&Fifo::write, fifo, message, size);
    std::thread receiver(&Fifo::read, fifo, output, size, true);
    sender.join();
    receiver.join();
    EXPECT_STREQ(message, output);
    delete fifo;
}
