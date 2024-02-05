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

#include <future>

#include "fifo.hpp"

#include <alchemy/pipe.h>
#include <alchemy/task.h>
#include <fcntl.h>
#include <poll.h>
#include <sys/eventfd.h>
#include <unistd.h>

#include "debug.hpp"

int FIFO_COUNT = 0;
constexpr std::string_view pipe_filesystem_prefix =
    "/proc/xenomai/registry/rtipc/xddp/";

// Generic xenomai fifo based on pipes
namespace RT::OS
{
class xenomaiFifo : public Fifo
{
public:
  explicit xenomaiFifo(size_t size);
  xenomaiFifo(const xenomaiFifo& fifo) = delete;
  xenomaiFifo& operator=(const xenomaiFifo& fifo) = delete;
  xenomaiFifo(xenomaiFifo&&) = default;
  xenomaiFifo& operator=(xenomaiFifo&&) = default;
  ~xenomaiFifo() override;

  int64_t read(void* buf, size_t buf_size) override;
  int64_t write(void* buf, size_t buf_size) override;
  int64_t readRT(void* buf, size_t buf_size) override;
  int64_t writeRT(void* buf, size_t buf_size) override;
  void poll() override;
  void close() override;
  int buffer_fd() const;
  size_t getCapacity() override;

private:
  bool closed = false;
  std::string pipe_name;
  int fd;  // file descriptor for non-realtime reading and writing
  int close_event_fd;
  int pipe_number;
  RT_PIPE pipe_handle {};
  size_t fifo_capacity;
  std::array<struct pollfd, 2> xbuf_poll_fd {};
};
}  // namespace RT::OS

RT::OS::xenomaiFifo::xenomaiFifo(size_t size)
    : pipe_name(std::string("RTXI-pipe-") + std::to_string(FIFO_COUNT++))
    , fifo_capacity(size)
{
  pipe_number = rt_pipe_create(
      &pipe_handle, pipe_name.c_str(), P_MINOR_AUTO, fifo_capacity);
  if (pipe_number < 0) {
    ERROR_MSG("RT::OS::xenomaiFifo : Unable to open real-time X pipe");
    ERROR_MSG("errno: {}", pipe_number);
    return;
  }
  const std::string filename = std::string(pipe_filesystem_prefix) + pipe_name;
  this->fd = ::open(filename.c_str(), O_RDWR | O_NONBLOCK);

  this->xbuf_poll_fd[0].fd = this->fd;
  this->xbuf_poll_fd[0].events = POLLIN;
  this->close_event_fd = eventfd(0, EFD_NONBLOCK);
  this->xbuf_poll_fd[1].fd = this->close_event_fd;
  this->xbuf_poll_fd[1].events = POLLIN;
}

RT::OS::xenomaiFifo::~xenomaiFifo()
{
  if (::close(this->fd) != 0) {
    ERROR_MSG("Unable to close non-realtime side of X pipe");
  }
  if (rt_pipe_delete(&this->pipe_handle) < 0) {
    ERROR_MSG("Unable to close real-time side of X pipe");
  }
}

int64_t RT::OS::xenomaiFifo::read(void* buf, size_t buf_size)
{
  // We need to specify to compiler that we are using read from c lib
  return ::read(this->fd, buf, buf_size);
}

int64_t RT::OS::xenomaiFifo::write(void* buf, size_t buf_size)
{
  // we need to specify to compiler that we are using write from c lib
  return ::write(this->fd, buf, buf_size);
}

int64_t RT::OS::xenomaiFifo::readRT(void* buf, size_t buf_size)
{
  return rt_pipe_read(&this->pipe_handle, buf, buf_size, TM_NONBLOCK);
}

int64_t RT::OS::xenomaiFifo::writeRT(void* buf, size_t buf_size)
{
  return rt_pipe_write(&this->pipe_handle, buf, buf_size, P_NORMAL);
}

void RT::OS::xenomaiFifo::poll()
{
  int errcode = ::poll(this->xbuf_poll_fd.data(), 2, -1);
  if (errcode < 0) {
    ERROR_MSG("RT::OS::FIFO(xenomai)::poll : returned with failure code {} : ",
              errcode);
    ERROR_MSG("{}", strerror(errcode));
  } else if ((this->xbuf_poll_fd[1].revents & POLLIN) != 0) {
    this->closed = true;
  }
}

int RT::OS::xenomaiFifo::buffer_fd() const
{
  return this->fd;
}

void RT::OS::xenomaiFifo::close()
{
  std::array<int64_t, 1> buf {};
  buf[0] = 1;
  ::write(this->close_event_fd, buf.data(), sizeof(int64_t));
}

size_t RT::OS::xenomaiFifo::getCapacity()
{
  return this->fifo_capacity;
}

int RT::OS::getFifo(std::unique_ptr<Fifo>& fifo, size_t fifo_size)
{
  // We can only create rt pipes from a xenomai thread.
  auto create_pipe_task = [&]()
  {
    rt_task_shadow(nullptr, "create-pipe-task", 0, 0);
    fifo = std::make_unique<RT::OS::xenomaiFifo>(fifo_size);
  };
  std::future task_future = std::async(std::launch::async, create_pipe_task);
  task_future.wait();
  return 0;
}
