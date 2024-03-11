/*
         The Real-Time eXperiment Interface (RTXI)
         Copyright (C) 2011 Georgia Institute of Technology, University of Utah,
   Will Cornell Medical College

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

#include <condition_variable>
#include <mutex>
#include <thread>

#include "fifo_tests.hpp"

#include "rtos.hpp"
#include "fifo.hpp"

TEST_F(FifoTest, getFifo)
{
  std::unique_ptr<RT::OS::Fifo> fifo;
  const size_t bufsize = this->default_buffer_size;
  const int result = RT::OS::getFifo(fifo, bufsize);
  ASSERT_EQ(result, 0);
  EXPECT_EQ(fifo->getCapacity(), bufsize);
}

TEST_F(FifoTest, roundtrip)
{
  std::unique_ptr<RT::OS::Fifo> fifo;
  const int result = RT::OS::getFifo(fifo, this->default_buffer_size);
  ASSERT_EQ(result, 0);
  auto task = std::make_unique<RT::OS::Task>();
  auto message_size = this->default_message.size();
  std::string output;
  output.reserve(this->default_message.size());
  auto echo = [&fifo](RT::OS::Task* tsk, size_t bufsize)
  {
    std::string buf;
    buf.reserve(bufsize);
    RT::OS::initiate(tsk);
    fifo->readRT(buf.data(), bufsize);
    fifo->writeRT(buf.data(), bufsize);
    RT::OS::shutdown(tsk);
  };
  fifo->write(this->default_message.data(), message_size);
  std::thread test_thread(echo, task.get(), message_size);
  test_thread.join();
  fifo->read(output.data(), message_size);
  EXPECT_STREQ(this->default_message.c_str(), output.c_str());
}

TEST_F(FifoTest, nonblocking)
{
  std::unique_ptr<RT::OS::Fifo> fifo;
  std::mutex mut;
  std::condition_variable cv;
  bool ready = false;

  // size_t size = this->default_buffer_size;
  std::string output;
  output.reserve(this->default_message.size());
  const int result = RT::OS::getFifo(fifo, this->default_buffer_size);
  ASSERT_EQ(result, 0);
  auto test_task = std::make_unique<RT::OS::Task>();
  RT::OS::setPeriod(test_task.get(), RT::OS::SECONDS_TO_NANOSECONDS);
  auto sender = [&]()
  {
    RT::OS::initiate(test_task.get());
    fifo->writeRT(this->default_message.data(),
                  this->default_message.size() * sizeof(char));
    std::unique_lock<std::mutex> lk(mut);
    ready = true;
    lk.unlock();
    cv.notify_one();
    RT::OS::shutdown(test_task.get());
  };

  std::thread sender_thread(sender);
  {
    std::unique_lock<std::mutex> lk(mut);
    cv.wait(lk, [&ready]() { return ready; });
  }
  const int64_t read_bytes =
      fifo->read(output.data(), this->default_message.size() * sizeof(char));
  sender_thread.join();

  ASSERT_NE(read_bytes, -1);
  ASSERT_STREQ(this->default_message.c_str(), output.c_str());
}
