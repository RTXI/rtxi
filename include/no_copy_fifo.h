#ifndef NO_COPY_FIFO_H
#define NO_COPY_FIFO_H

#include <pthread.h>

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
    volatile size_t rptr;
    volatile size_t wptr;
    size_t rsize;
    size_t wsize;
    size_t size;
    volatile size_t wrap;

    pthread_mutex_t mutex;
    pthread_cond_t data_available;

}; // NoCopyFifo

#endif // NO_COPY_FIFO_H
