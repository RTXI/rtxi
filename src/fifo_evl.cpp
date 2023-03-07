

#include "fifo.hpp"

#include <errno.h>
#include <evl/evl.h>
#include <string.h>
#include <unistd.h>

#include "debug.hpp"

// First let's define an evl specific Fifo class.
namespace RT::OS
{
class evlFifo : public Fifo
{
public:
  explicit evlFifo(size_t size);
  evlFifo(const evlFifo& fifo) = delete;  // copy constructor
  evlFifo& operator=(const evlFifo& fifo) = delete;  // copy assignment operator
  evlFifo(evlFifo&&) = default;  // move constructor
  evlFifo& operator=(evlFifo&&) = default;  // move assignment operator
  ~evlFifo() override;

  int buffer_fd() const;
  ssize_t read(void* buf, size_t buf_size) override;
  ssize_t write(void* buf, size_t buf_size) override;
  ssize_t readRT(void* buf, size_t buf_size) override;
  ssize_t writeRT(void* buf, size_t buf_size) override;
  size_t getCapacity() override;

private:
  int xbuf_fd;
  size_t fifo_capacity;
};
}  // namespace RT::OS

RT::OS::evlFifo::evlFifo(size_t size)
    : fifo_capacity(size)
{
  this->xbuf_fd = evl_create_xbuf(fifo_capacity,
                                  fifo_capacity,
                                  EVL_CLONE_PRIVATE | EVL_CLONE_NONBLOCK,
                                  "RTXI Fifo");
}

RT::OS::evlFifo::~evlFifo()
{
  ::close(this->xbuf_fd);
}

ssize_t RT::OS::evlFifo::read(void* buf, size_t buf_size)
{
  // We need to specify to compiler that we are using read from c lib
  return ::read(this->xbuf_fd, buf, buf_size);
}

ssize_t RT::OS::evlFifo::write(void* buf, size_t buf_size)
{
  // we need to specify to compiler that we are using write from c lib
  return ::write(this->xbuf_fd, buf, buf_size);
}

ssize_t RT::OS::evlFifo::readRT(void* buf, size_t buf_size)
{
  return oob_read(this->xbuf_fd, buf, buf_size);
}

ssize_t RT::OS::evlFifo::writeRT(void* buf, size_t buf_size)
{
  return oob_write(this->xbuf_fd, buf, buf_size);
}

int RT::OS::evlFifo::buffer_fd() const
{
  return this->xbuf_fd;
}

size_t RT::OS::evlFifo::getCapacity()
{
  return this->fifo_capacity;
}

int RT::OS::getFifo(std::unique_ptr<Fifo>& fifo, size_t fifo_size)
{
  int init_state = 0;
  auto tmp_fifo = std::make_unique<RT::OS::evlFifo>(fifo_size);
  if (tmp_fifo->buffer_fd() < 0) {
    init_state = tmp_fifo->buffer_fd();
    ERROR_MSG("RT::OS::getFifo(EVL) : {}", strerror(errno));
  } else {
    fifo = std::move(tmp_fifo);
  }
  return init_state;
}
