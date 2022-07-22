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

#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#include <array>

#include "debug.hpp"
#include "fifo.hpp"

// Generic posix fifo based on pipes
namespace RT::OS
{
class posixFifo : public RT::OS::Fifo 
{
  public:
  explicit posixFifo(size_t size);
  posixFifo(const posixFifo& fifo) = delete;
  posixFifo& operator=(const posixFifo& fifo) = delete;
  posixFifo(posixFifo &&) = default;
  posixFifo& operator=(posixFifo &&) = default;
  ~posixFifo() override;

  size_t getCapacity() override;
  ssize_t read(void* buf, size_t buf_size) override;
  ssize_t write(void* buf, size_t buf_size) override;
  ssize_t readRT(void* buf, size_t buf_size) override;
  ssize_t writeRT(void* buf, size_t buf_size) override;

  private:
  std::array<int, 2> rt_to_ui_fd; // 0 is read from rt; 1 is write to ui
  std::array<int, 2> ui_to_rt_fd; // 0 is read from ui; 1 is write to rt
  size_t fifo_capacity;
};
}  // namespace RT::OS

RT::OS::posixFifo::posixFifo(size_t size) : rt_to_ui_fd({0,1}),
                                            ui_to_rt_fd({0,1}),
                                            fifo_capacity(size)
{
  char buf[256]; // NOLINT: we have to use c arrays with c functions
  char *err = nullptr;
  if (pipe2(this->rt_to_ui_fd.data(), O_NONBLOCK | O_CLOEXEC) != 0){
    ERROR_MSG("RT::OS::posixFifo : failed to create rt-to-ui ipc : {}\n{}", err, buf); // NOLINT
  }
  if (pipe2(this->ui_to_rt_fd.data(), O_NONBLOCK | O_CLOEXEC) != 0){
    ERROR_MSG("RT::OS::posixFifo : failed to create ui-to-rt ipc : {}\n{}", err, buf); // NOLINT
  }
}

RT::OS::posixFifo::~posixFifo()
{
  ::close(rt_to_ui_fd[0]);
  ::close(rt_to_ui_fd[1]);
  ::close(ui_to_rt_fd[0]);
  ::close(ui_to_rt_fd[1]);
}

ssize_t RT::OS::posixFifo::read(void* buf, size_t buf_size)
{
  // We need to specify to compiler that we are using read from c lib
  return ::read(this->rt_to_ui_fd[0], buf, buf_size);
}

ssize_t RT::OS::posixFifo::write(void* buf, size_t buf_size)
{
  // we need to specify to compiler that we are using write from c lib
  return ::write(this->ui_to_rt_fd[1], buf, buf_size);
}

ssize_t RT::OS::posixFifo::readRT(void* buf, size_t buf_size)
{
  return ::read(this->ui_to_rt_fd[0] , buf, buf_size);
}

ssize_t RT::OS::posixFifo::writeRT(void* buf, size_t buf_size)
{
  return ::write(this->rt_to_ui_fd[1], buf, buf_size);
}

size_t RT::OS::posixFifo::getCapacity()
{
  return this->fifo_capacity;
}

int RT::OS::getFifo(std::unique_ptr<Fifo>& fifo, size_t fifo_size)
{
  auto tmp_fifo = std::make_unique<RT::OS::posixFifo>(fifo_size);
  fifo = std::move(tmp_fifo);
  return 0;
}
