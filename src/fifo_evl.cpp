

#include "fifo.hpp"

#include <errno.h>
#include <evl/evl.h>
#include <poll.h>
#include <string.h>
#include <sys/eventfd.h>
#include <sys/poll.h>
#include <unistd.h>

#include "debug.hpp"

int FIFO_COUNT = 0;

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
  int64_t read(void* buf, size_t buf_size) override;
  int64_t write(void* buf, size_t buf_size) override;
  int64_t readRT(void* buf, size_t buf_size) override;
  int64_t writeRT(void* buf, size_t buf_size) override;
  void poll() override;
  void close() override;
  size_t getCapacity() override;

private:
  int xbuf_fd;
  int close_event_fd;
  size_t fifo_capacity;
  std::array<struct pollfd, 2> xbuf_poll_fd {};
  bool closed = false;
};
}  // namespace RT::OS

RT::OS::evlFifo::evlFifo(size_t size)
    : fifo_capacity(size)
{
  this->xbuf_fd = evl_create_xbuf(fifo_capacity,
                                  fifo_capacity,
                                  EVL_CLONE_PRIVATE | EVL_CLONE_NONBLOCK,
                                  "RTXI Fifo %d",
                                  FIFO_COUNT++);
  if (this->xbuf_fd <= 0) {
    ERROR_MSG("RT::OS::FIFO(evl) : Unable to create real-time buffer\n");
    ERROR_MSG("evl core : {}", strerror(this->xbuf_fd));
    return;
  }
  this->xbuf_poll_fd[0].fd = this->xbuf_fd;
  this->xbuf_poll_fd[0].events = POLLIN;
  this->close_event_fd = eventfd(0, EFD_NONBLOCK);
  this->xbuf_poll_fd[1].fd = this->close_event_fd;
  this->xbuf_poll_fd[1].events = POLLIN;
}

RT::OS::evlFifo::~evlFifo()
{
  ::close(this->xbuf_fd);
}

int64_t RT::OS::evlFifo::read(void* buf, size_t buf_size)
{
  // We need to specify to compiler that we are using read from c lib
  return ::read(this->xbuf_fd, buf, buf_size);
}

int64_t RT::OS::evlFifo::write(void* buf, size_t buf_size)
{
  // we need to specify to compiler that we are using write from c lib
  return ::write(this->xbuf_fd, buf, buf_size);
}

int64_t RT::OS::evlFifo::readRT(void* buf, size_t buf_size)
{
  return oob_read(this->xbuf_fd, buf, buf_size);
}

int64_t RT::OS::evlFifo::writeRT(void* buf, size_t buf_size)
{
  return oob_write(this->xbuf_fd, buf, buf_size);
}

void RT::OS::evlFifo::poll()
{
  int errcode = ::poll(this->xbuf_poll_fd.data(), 2, -1);
  if (errcode < 0) {
    ERROR_MSG("RT::OS::FIFO(evl)::poll : returned with failure code {} : ",
              errcode);
    ERROR_MSG("{}", strerror(errcode));
  } else if ((this->xbuf_poll_fd[1].revents & POLLIN) != 0) {
    this->closed = true;
  }
}

int RT::OS::evlFifo::buffer_fd() const
{
  return this->xbuf_fd;
}

void RT::OS::evlFifo::close()
{
  std::array<int64_t, 1> buf {};
  buf[0] = 1;
  ::write(this->close_event_fd, buf.data(), sizeof(int64_t));
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
