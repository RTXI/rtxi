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

#include <functional>
#include <thread>

#include "fifo_tests.hpp"

#include "debug.hpp"
#include "rtos.hpp"

TEST_F(FifoTest, getFifo)
{
  size_t bufsize = this->default_buffer_size;
  int result = RT::OS::getFifo(fifo, bufsize);
  ASSERT_EQ(result, 0);
  EXPECT_EQ(fifo->getCapacity(), bufsize);
}

TEST_F(FifoTest, roundtrip)
{
  char output[this->default_buffer_size];
  size_t size = this->default_buffer_size;
  int result = RT::OS::getFifo(fifo, size);
  ASSERT_EQ(result, 0);
  auto echo = [this](size_t bufsize)
  {
    char buf[bufsize];
    RT::OS::initiate();
    this->fifo->readRT(&buf, bufsize, false);
    this->fifo->writeRT(&buf, bufsize);
    RT::OS::shutdown();
  };
  this->fifo->write(this->default_message, size);
  std::thread test_thread(echo, size);
  test_thread.join();
  this->fifo->read(output, size, false);
  EXPECT_STREQ(this->default_message, output);
}

TEST_F(FifoTest, nonblocking)
{
  size_t size = this->default_buffer_size;
  char output[this->default_message_size];
  int result = RT::OS::getFifo(fifo, size);
  auto test_task = std::make_unique<RT::OS::Task>();
  RT::OS::setPeriod(test_task, RT::OS::SECONDS_TO_NANOSECONDS);
  auto sender = [this, &test_task]()
  {
    RT::OS::initiate();
    RT::OS::sleepTimestep(test_task);
    this->fifo->writeRT(&(this->default_message), this->default_message_size);
    RT::OS::shutdown();
  };
  int64_t start_time = RT::OS::getTime();
  std::thread sender_thread(sender);
  fifo->read(&output, this->default_message_size, false);
  int64_t end_time = RT::OS::getTime();
  sender_thread.join();
  int64_t duration = end_time - start_time;
  // Should return immediately when in nonblocking mode. No message transferred
  ASSERT_STRNE(output, this->default_message);
  EXPECT_GE(test_task->period, duration);
}
