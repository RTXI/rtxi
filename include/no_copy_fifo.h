#ifndef NO_COPY_FIFO_H
#define NO_COPY_FIFO_H

#include <sem.h>

class NoCopyFifo {

public:

    NoCopyFifo(size_t);
    ~NoCopyFifo(void);

    void *read(size_t,bool =true);
    void readDone(void);

    void *write(size_t);
    void writeDone(void);

private:

    char *data;
    size_t rptr;
    size_t wptr;
    Semaphore sem;
    size_t rsize;
    size_t wsize;
    size_t size;
    size_t wrap;

}; // NoCopyFifo

#endif // NO_COPY_FIFO_H
