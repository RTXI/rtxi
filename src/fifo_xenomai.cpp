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

#include <filesystem>
#include <unistd.h>
#include <fcntl.h>
#include <alchemy/pipe.h>

#include "debug.hpp"
#include "fifo.hpp"

// Generic xenomai fifo based on pipes
namespace RT::OS
{
class xenomaiFifo : public RT::OS::Fifo 
{
  public:
  explicit xenomaiFifo(size_t size);
  xenomaiFifo(const xenomaiFifo& fifo) = delete;
  xenomaiFifo& operator=(const xenomaiFifo& fifo) = delete;
  xenomaiFifo(xenomaiFifo &&) = default;
  xenomaiFifo& operator=(xenomaiFifo &&) = default;
  ~xenomaiFifo() override;

  size_t getCapacity() override;
  ssize_t read(void* buf, size_t buf_size) override;
  ssize_t write(void* buf, size_t buf_size) override;
  ssize_t readRT(void* buf, size_t buf_size) override;
  ssize_t writeRT(void* buf, size_t buf_size) override;

  private:
  std::string pipeName;
  int fd; // file descriptor for non-realtime reading and writting
  RT_PIPE * pipe_ptr = nullptr;
  size_t fifo_capacity;
};
}  // namespace RT::OS

RT::OS::xenomaiFifo::xenomaiFifo(size_t size)
{
  bool pipename_not_found = true;
  std::string pipename_prefix = "/proc/xenomai/registry/rtipc/xddp/RTXIPipe";
  int pipe_number = 0;
  while (pipe_number < 1000){
    pipename_not_found = std::filesystem::exists(pipename_prefix + std::to_string(pipe_number));
    if(!pipename_not_found){ break; }
    pipe_number++;
  }
  this->pipeName = pipename_prefix + std::to_string(pipe_number);
  if(!rt_pipe_create(this->pipe_ptr, this->pipeName.c_str(), P_MINOR_AUTO, size)){
    ERROR_MSG("Unable to open real-time X pipe");
  } else {
    this->fd = ::open(this->pipeName.c_str(), O_RDWR);
  }
}

RT::OS::xenomaiFifo::~xenomaiFifo()
{
  if(::close(this->fd) != 0){
    ERROR_MSG("Unable to close non-realtime side of X pipe");
  }
  if(!rt_pipe_delete(this->pipe_ptr)){
    ERROR_MSG("Unable to close real-time side of X pipe");
  }
}

ssize_t RT::OS::xenomaiFifo::read(void* buf, size_t buf_size)
{
  // We need to specify to compiler that we are using read from c lib
  return ::read(this->fd, buf, buf_size);
}

ssize_t RT::OS::xenomaiFifo::write(void* buf, size_t buf_size)
{
  // we need to specify to compiler that we are using write from c lib
  return ::write(this->fd, buf, buf_size);
}

ssize_t RT::OS::xenomaiFifo::readRT(void* buf, size_t buf_size)
{
  return rt_pipe_read(this->pipe_ptr, buf, buf_size, TM_NONBLOCK);
}

ssize_t RT::OS::xenomaiFifo::writeRT(void* buf, size_t buf_size)
{
  return rt_pipe_write(this->pipe_ptr, buf, buf_size, TM_NONBLOCK);
}

size_t RT::OS::xenomaiFifo::getCapacity()
{
  return this->fifo_capacity;
}

int RT::OS::getFifo(std::unique_ptr<Fifo>& fifo, size_t fifo_size)
{
  auto tmp_fifo = std::make_unique<RT::OS::xenomaiFifo>(fifo_size);
  fifo = std::move(tmp_fifo);
  return 0;
}
