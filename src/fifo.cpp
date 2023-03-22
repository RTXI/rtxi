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

#include <array>
#include <condition_variable>
#include <mutex>

#include "fifo.hpp"

#include "debug.hpp"

// Generic posix fifo based on pipes
namespace RT::OS
{
class xbuffFifo : public RT::OS::Fifo
{
public:
  explicit xbuffFifo(size_t size);
  xbuffFifo(const xbuffFifo& fifo) = delete;
  xbuffFifo& operator=(const xbuffFifo& fifo) = delete;
  xbuffFifo(xbuffFifo&&) = default;
  xbuffFifo& operator=(xbuffFifo&&) = default;
  ~xbuffFifo() override;

  size_t getCapacity() override;
  ssize_t read(void* buf, size_t data_size) override;
  ssize_t write(void* buf, size_t data_size) override;
  ssize_t readRT(void* buf, size_t data_size) override;
  ssize_t writeRT(void* buf, size_t data_size) override;
  void poll() override;

private:
  char* rt_to_ui;  // 0 is read from rt; 1 is write to ui
  char* ui_to_rt;  // 0 is read from ui; 1 is write to rt
  const uint64_t fifo_capacity;
  std::mutex mutex_rt2ui;
  std::mutex mutex_ui2rt;
  uint64_t ui_rptr = 0;
  uint64_t ui_wptr = 0;
  uint64_t rt_rptr = 0;
  uint64_t rt_wptr = 0;
  uint64_t ui_available_read_bytes;
  uint64_t ui_available_write_bytes;
  uint64_t rt_available_read_bytes;
  uint64_t rt_available_write_bytes;

  // synchronization primitives specific to poll function
  std::mutex ui_read_mut;
  std::condition_variable available_read_cond;
};
}  // namespace RT::OS

RT::OS::xbuffFifo::xbuffFifo(size_t size)
    : fifo_capacity(size)
{
  rt_to_ui = new char[this->fifo_capacity];
  ui_to_rt = new char[this->fifo_capacity];
}

RT::OS::xbuffFifo::~xbuffFifo()
{
  delete[] rt_to_ui;
  delete[] ui_to_rt;
}

ssize_t RT::OS::xbuffFifo::read(void* buf, size_t data_size)
{
  if (rt_wptr == ui_rptr) {
    return -1;
  }
  std::unique_lock<std::mutex> lock(mutex_rt2ui);
  // return if the caller requests more data than available
  if (ui_available_read_bytes < data_size) {
    return -1;
  }
  if (fifo_capacity - ui_rptr < data_size) {
    uint64_t m = fifo_capacity - ui_rptr;
    memcpy(buf, rt_to_ui + ui_rptr, m);
    memcpy(reinterpret_cast<char*>(buf) + m, rt_to_ui, data_size - m);
  } else {
    memcpy(buf, rt_to_ui + ui_rptr, data_size);
  }
  ui_rptr = (ui_rptr + data_size) % this->fifo_capacity;
  ui_available_read_bytes = (fifo_capacity + ui_rptr - rt_wptr) % fifo_capacity;
  return static_cast<ssize_t>(data_size);
}

ssize_t RT::OS::xbuffFifo::write(void* buf, size_t data_size)
{
  if (data_size > fifo_capacity - rt_available_read_bytes) {
    ERROR_MSG("FIFO::write : Fifo full, data lost\n");
    return -1;
  }

  if (data_size > fifo_capacity - ui_wptr) {
    uint64_t m = fifo_capacity - ui_wptr;
    memcpy(ui_to_rt + ui_wptr, buf, m);
    memcpy(ui_to_rt, reinterpret_cast<const char*>(buf) + m, data_size - m);
  } else {
    memcpy(ui_to_rt + ui_wptr, buf, data_size);
  }
  ui_wptr = (ui_wptr + data_size) % fifo_capacity;
  rt_available_read_bytes = (fifo_capacity + rt_rptr - ui_wptr) % fifo_capacity;
  return static_cast<ssize_t>(data_size);
}

ssize_t RT::OS::xbuffFifo::readRT(void* buf, size_t data_size)
{
  if (ui_wptr == rt_rptr) {
    return -1;
  }
  std::unique_lock<std::mutex> lock(mutex_rt2ui);
  // return if the caller requests more data than available
  if (rt_available_read_bytes < data_size) {
    return -1;
  }
  if (fifo_capacity - rt_rptr < data_size) {
    uint64_t m = fifo_capacity - rt_rptr;
    memcpy(buf, ui_to_rt + rt_rptr, m);
    memcpy(reinterpret_cast<char*>(buf) + m, ui_to_rt, data_size - m);
  } else {
    memcpy(buf, ui_to_rt + rt_rptr, data_size);
  }
  rt_rptr = (rt_rptr + data_size) % this->fifo_capacity;
  rt_available_read_bytes = (fifo_capacity + rt_rptr - ui_wptr) % fifo_capacity;
  return static_cast<ssize_t>(data_size);
}

ssize_t RT::OS::xbuffFifo::writeRT(void* buf, size_t data_size)
{
  if (data_size > fifo_capacity - ui_available_read_bytes) {
    ERROR_MSG("FIFO::write : Fifo full, data lost\n");
    return -1;
  }

  if (data_size > fifo_capacity - rt_wptr) {
    uint64_t m = fifo_capacity - rt_wptr;
    memcpy(rt_to_ui + rt_wptr, buf, m);
    memcpy(rt_to_ui, reinterpret_cast<const char*>(buf) + m, data_size - m);
  } else {
    memcpy(rt_to_ui + rt_wptr, buf, data_size);
  }
  rt_wptr = (rt_wptr + data_size) % fifo_capacity;
  ui_available_read_bytes = (fifo_capacity + ui_rptr - rt_wptr) % fifo_capacity;
  this->available_read_cond.notify_one();
  return static_cast<ssize_t>(data_size);
}

void RT::OS::xbuffFifo::poll()
{
  std::unique_lock<std::mutex> lk(this->ui_read_mut);
  if(ui_available_read_bytes == 0) {
    this->available_read_cond.wait(
      lk, 
      [this](){return ui_available_read_bytes != 0;}
    );
  }
}

size_t RT::OS::xbuffFifo::getCapacity()
{
  return this->fifo_capacity;
}

int RT::OS::getFifo(std::unique_ptr<Fifo>& fifo, size_t fifo_size)
{
  auto tmp_fifo = std::make_unique<RT::OS::xbuffFifo>(fifo_size);
  fifo = std::move(tmp_fifo);
  return 0;
}
