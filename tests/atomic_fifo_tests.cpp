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

#include <atomic_fifo_tests.h>
#include <thread>


TEST_F(AtomicFifoTest, isLockFree)
{
    fifo = new AtomicFifo((size_t) 100);
    ASSERT_TRUE(fifo->isLockFree());
    delete fifo;
}

TEST_F(AtomicFifoTest, ReadAndWrite)
{
    fifo = new AtomicFifo((size_t) 100); 
    char inbuff[22];
    char outbuff[22];
    for(int i=0; i<9; i++){
        inbuff[i] = (char) i+97;
    }
    inbuff[9] = '\0';

    // There should be zero bits read if the fifo is empty
    EXPECT_EQ((size_t) 0, fifo->read(outbuff, (size_t) 10));

    // check that read and write work properly
    bool write_success = fifo->write(inbuff, (size_t) 21);
    bool read_success = fifo->read(outbuff, (size_t) 21);
    EXPECT_STREQ(inbuff, outbuff);
    EXPECT_EQ(write_success, read_success);

    // The fifo should be empty after reading
    EXPECT_EQ((size_t) 0, fifo->read(outbuff, (size_t) 21));

    // should be able to write multiple times
    for(int i = 0; i < 10; i++){
        write_success = fifo->write(inbuff, (size_t) 11);
        read_success = fifo->read(outbuff, (size_t) 11);
        EXPECT_STREQ(inbuff, outbuff);
        EXPECT_EQ(write_success, read_success);
    }

    delete fifo;
}

TEST_F(AtomicFifoTest, Failures)
{
    fifo = new AtomicFifo((size_t) 2);
    char buff[8] = "message";

    // Test whether FIFO fails when overwritting to it
    EXPECT_EQ(fifo->write(buff, (size_t) 8), (size_t) 0);
}

void send(std::mutex &m, std::condition_variable &cv, std::atomic<bool> &ready, AtomicFifo *fifo, char *message, size_t size)
{
    std::unique_lock<std::mutex> lk(m);
    fifo->write(message, size);
    ready.store(true);
    cv.notify_one();
}

void receive(std::mutex &m, std::condition_variable &cv, std::atomic<bool> &ready, AtomicFifo *fifo, char *output, size_t size)
{
    std::unique_lock<std::mutex> lk(m);
    // handle spurius calls
    cv.wait(lk, [&]{return ready.load();});

    fifo->read(output, size);
}

TEST_F(AtomicFifoTest, Threaded)
{
    std::mutex m;
    std::condition_variable cv;
    std::atomic<bool> ready;
    ready.store(false);

    char message[8] = "message";
    char output[8]; 
    size_t size = 8;
    fifo = new AtomicFifo((size_t) 10);
    std::thread sender(&send, std::ref(m), std::ref(cv), std::ref(ready), fifo, message, size);
    std::thread receiver(&receive, std::ref(m), std::ref(cv), std::ref(ready), fifo, output, size);
    sender.join();
    receiver.join();
    EXPECT_STREQ(message, output);
    delete fifo;
}

