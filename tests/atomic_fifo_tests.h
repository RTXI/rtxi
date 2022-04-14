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

#ifndef ATOMIC_FIFO_TESTS_H
#define ATOMIC_FIFO_TESTS_H

#include <gtest/gtest.h>
#include <atomic_fifo.h>

// C++20 has a semaphore library, but can't use it just yet as it's too new and might not
// work for current users of RTXI testing facilities. Use condition variable and mutex instead. 
#include <condition_variable>
#include <mutex>

// Define all fixtures for testing purposes
class AtomicFifoTest : public ::testing::Test
{
protected:
    AtomicFifoTest() { }
    ~AtomicFifoTest() { }

    AtomicFifo *fifo;
};

// Need wrappers on fifo methods for threadding purposes
void send(std::mutex &m, std::condition_variable &cv, std::atomic<bool> &ready, AtomicFifo *fifo, char *message, size_t size);
void receive(std::mutex &m, std::condition_variable &cv, std::atomic<bool> &ready, AtomicFifo *fifo, char *output, size_t size);

#endif
