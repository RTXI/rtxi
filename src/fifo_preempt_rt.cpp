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

#include "fifo.hpp"

#include <fcntl.h>
#include <sys/eventfd.h>
#include <sys/mman.h>
#include <sys/poll.h>
#include <unistd.h>

#include "debug.hpp"

// Generic posix fifo based on pipes
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
  std::array<int, 2> ui_to_rt {};
  std::array<int, 2> rt_to_ui {};

  size_t fifo_capacity = 0;
  int close_event_fd;
  std::array<struct pollfd, 2> xbuf_poll_fd {};
  bool closed = false;
  int errcode = 0;
};
}  // namespace RT::OS

RT::OS::preemptRTFifo::preemptRTFifo(size_t size)
    : fifo_capacity(size)
    , errcode(pipe2(this->rt_to_ui.data(), O_CLOEXEC))
{
  if (this->errcode != 0) {
    ERROR_MSG("RT::OS::preemptRTFifo : Unable to create RT to UI buffer");
    return;
  }
  this->errcode = pipe2(this->ui_to_rt.data(), O_CLOEXEC);
  if (this->errcode != 0) {
    ERROR_MSG("RT::OS::preemptRTFifo : Unable to create UI to RT buffer");
    return;
  }
  // Make sure the reads/writes are nonblock to match api behavior
  fcntl(this->rt_to_ui[0], F_SETFL, O_NONBLOCK);
  fcntl(this->rt_to_ui[1], F_SETFL, O_NONBLOCK);
  fcntl(this->ui_to_rt[0], F_SETFL, O_NONBLOCK);
  fcntl(this->rt_to_ui[1], F_SETFL, O_NONBLOCK);

  // We can set the size of the pipe. Again we try to match api behavior
  fcntl(this->rt_to_ui[1], F_SETPIPE_SZ, size);
  fcntl(this->ui_to_rt[1], F_SETPIPE_SZ, size);

  this->xbuf_poll_fd[0].fd = this->rt_to_ui[0];
  this->xbuf_poll_fd[0].events = POLLIN;
  this->close_event_fd = eventfd(0, EFD_NONBLOCK);
  this->xbuf_poll_fd[1].fd = this->close_event_fd;
  this->xbuf_poll_fd[1].events = POLLIN;
}

RT::OS::preemptRTFifo::~preemptRTFifo()
{
  ::close(rt_to_ui[0]);
  ::close(rt_to_ui[1]);
  ::close(ui_to_rt[0]);
  ::close(ui_to_rt[1]);
}

int64_t RT::OS::preemptRTFifo::read(void* buf, size_t data_size)
{
  return ::read(rt_to_ui[0], buf, data_size);
}

int64_t RT::OS::preemptRTFifo::write(void* buf, size_t data_size)
{
  return ::write(ui_to_rt[1], buf, data_size);
}

int64_t RT::OS::preemptRTFifo::readRT(void* buf, size_t data_size)
{
  return ::read(ui_to_rt[0], buf, data_size);
}

int64_t RT::OS::preemptRTFifo::writeRT(void* buf, size_t data_size)
{
  return ::write(rt_to_ui[1], buf, data_size);
}

void RT::OS::preemptRTFifo::poll()
{
  this->errcode = ::poll(this->xbuf_poll_fd.data(), 2, -1);
  if (errcode < 0) {
    std::string errbuff(255, '\0');
    ERROR_MSG(
        "RT::OS::FIFO(PREMPT_RT)::poll : returned with failure code {} : ",
        errcode);
    ERROR_MSG("{}", strerror_r(this->errcode, errbuff.data(), errbuff.size()));
  } else if ((this->xbuf_poll_fd[1].revents & POLLIN) != 0) {
    this->closed = true;
  }
}

void RT::OS::preemptRTFifo::close()
{
  std::array<int64_t, 1> buf = {1};
  this->closed = true;
  ::write(this->close_event_fd, buf.data(), sizeof(int64_t));
}

size_t RT::OS::preemptRTFifo::getCapacity()
{
  return this->fifo_capacity;
}

int RT::OS::preemptRTFifo::getErrorCode() const
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
