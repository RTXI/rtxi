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

#include <algorithm>
#include <array>
#include <cstring>
#include <memory>

#include "fifo.hpp"

#include <sys/eventfd.h>
#include <sys/poll.h>
#include <unistd.h>

#include "debug.hpp"

namespace RT::OS
{

class preemptRTFifo : public RT::OS::Fifo
{
public:
  explicit preemptRTFifo(size_t size);
  preemptRTFifo(const preemptRTFifo& fifo) = delete;
  preemptRTFifo& operator=(const preemptRTFifo& fifo) = delete;
  preemptRTFifo(preemptRTFifo&&) = default;
  preemptRTFifo& operator=(preemptRTFifo&&) = default;
  ~preemptRTFifo() override;

  size_t getCapacity() override;
  int64_t read(void* buf, size_t data_size) override;
  int64_t write(void* buf, size_t data_size) override;
  int64_t readRT(void* buf, size_t data_size) override;
  int64_t writeRT(void* buf, size_t data_size) override;
  void poll() override;
  void close() override;
  int getErrorCode() const;

private:
  static constexpr size_t ENTRY_SIZE = 256;
  static constexpr size_t RING_CAPACITY = 1024;

  struct Entry
  {
    uint8_t data[ENTRY_SIZE];
    size_t size;
  };

  class RingBuffer
  {
  public:
    bool push(const Entry& e)
    {
      const size_t next = (head + 1) % RING_CAPACITY;
      if (next == tail)
        return false;
      buffer[head] = e;
      head = next;
      return true;
    }

    bool pop(Entry& e)
    {
      if (head == tail)
        return false;
      e = buffer[tail];
      tail = (tail + 1) % RING_CAPACITY;
      return true;
    }

    bool empty() const { return head == tail; }

  private:
    std::array<Entry, RING_CAPACITY> buffer {};
    size_t head = 0;
    size_t tail = 0;
  };

  RingBuffer rt_to_ui_ring;
  RingBuffer ui_to_rt_ring;

  size_t fifo_capacity;
  int close_event_fd;
  std::array<struct pollfd, 1> xbuf_poll_fd {};
  bool closed = false;
  int errcode = 0;
};

}  // namespace RT::OS

using namespace RT::OS;

preemptRTFifo::preemptRTFifo(size_t size)
    : fifo_capacity(size)
{
  this->close_event_fd = eventfd(0, EFD_NONBLOCK);
  if (close_event_fd == -1) {
    ERROR_MSG("RT::OS::preemptRTFifo : eventfd creation failed");
    errcode = errno;
    return;
  }

  this->xbuf_poll_fd[0].fd = this->close_event_fd;
  this->xbuf_poll_fd[0].events = POLLIN;
}

preemptRTFifo::~preemptRTFifo()
{
  ::close(close_event_fd);
}

int64_t preemptRTFifo::writeRT(void* buf, size_t data_size)
{
  Entry e;
  if (data_size > ENTRY_SIZE)
    return -1;
  std::memcpy(e.data, buf, data_size);
  e.size = data_size;

  if (!rt_to_ui_ring.push(e))
    return 0;

  uint64_t notify = 1;
  ::write(close_event_fd, &notify, sizeof(notify));
  return static_cast<int64_t>(data_size);
}

int64_t preemptRTFifo::read(void* buf, size_t data_size)
{
  Entry e;
  if (!rt_to_ui_ring.pop(e))
    return 0;

  std::memcpy(buf, e.data, std::min(data_size, e.size));
  return static_cast<int64_t>(std::min(data_size, e.size));
}

int64_t preemptRTFifo::write(void* buf, size_t data_size)
{
  Entry e;
  if (data_size > ENTRY_SIZE)
    return -1;
  std::memcpy(e.data, buf, data_size);
  e.size = data_size;

  if (!ui_to_rt_ring.push(e))
    return 0;
  return static_cast<int64_t>(data_size);
}

int64_t preemptRTFifo::readRT(void* buf, size_t data_size)
{
  Entry e;
  if (!ui_to_rt_ring.pop(e))
    return 0;

  std::memcpy(buf, e.data, std::min(data_size, e.size));
  return static_cast<int64_t>(std::min(data_size, e.size));
}

void preemptRTFifo::poll()
{
  this->errcode = ::poll(this->xbuf_poll_fd.data(), 1, -1);
  if (errcode < 0) {
    std::string errbuff(255, '\0');
    ERROR_MSG(
        "RT::OS::FIFO(preempt_rt)::poll : returned with failure code {} : ",
        errcode);
    ERROR_MSG("{}", strerror_r(this->errcode, errbuff.data(), errbuff.size()));
  }
}

void preemptRTFifo::close()
{
  std::array<int64_t, 1> buf = {1};
  this->closed = true;
  ::write(this->close_event_fd, buf.data(), sizeof(int64_t));
}

size_t preemptRTFifo::getCapacity()
{
  return this->fifo_capacity;
}

int preemptRTFifo::getErrorCode() const
{
  return this->errcode;
}

int RT::OS::getFifo(std::unique_ptr<Fifo>& fifo, size_t fifo_size)
{
  auto tmp_fifo = std::make_unique<RT::OS::preemptRTFifo>(fifo_size);
  const int errcode = tmp_fifo->getErrorCode();
  if (errcode != 0) {
    std::string errbuff(255, '\0');
    ERROR_MSG("RT::OS::preemptRTFifo : {}",
              strerror_r(errcode, errbuff.data(), errbuff.size()));
  } else {
    fifo = std::move(tmp_fifo);
  }
  return errcode;
}
