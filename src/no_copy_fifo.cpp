#include <debug.h>
#include <no_copy_fifo.h>

#define MAX(a,b)     ((a > b) ? (a) : (b))
#define AVAILABLE    ((wrap+wptr-rptr)%wrap)

NoCopyFifo::NoCopyFifo(size_t s)
    : rptr(0), wptr(0), rsize(0), wsize(0), size(s), wrap(s) {
    data = new char[size];

    pthread_mutex_init(&mutex,NULL);
    pthread_cond_init(&data_available,NULL);
}

NoCopyFifo::~NoCopyFifo(void) {
    delete[] data;

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&data_available);
}

void *NoCopyFifo::read(size_t n,bool blocking) {
    if(blocking)
        pthread_mutex_lock(&mutex);
    else if(pthread_mutex_trylock(&mutex) != 0)
        return NULL;

    if(AVAILABLE < n)
        if(blocking) {
            do {
                pthread_cond_wait(&data_available,&mutex);
            } while(AVAILABLE < n);
        } else {
            pthread_mutex_unlock(&mutex);
            return NULL;
        }

    rsize = n;
    void *ptr = data+rptr;

    pthread_mutex_unlock(&mutex);

    return ptr;
}

void NoCopyFifo::readDone(void) {
    rptr += rsize;

    if(rptr >= wrap)
        rptr = 0;

    rsize = 0;
}

void *NoCopyFifo::write(size_t n) {
    size_t available;
    if(rptr <= wptr)
        available = MAX(size-wptr,rptr);
    else
        available = rptr - wptr - 1;

    if(n >= available) {
        ERROR_MSG("NoCopyFifo::write : buffer full, data is being lost\n");
        return NULL;
    }

    if(rptr <= wptr)
        if(size-wptr < n) {
            wptr = 0;
            wrap = wptr;
        }

    wsize = n;
    return data+wptr;
}

void NoCopyFifo::writeDone(void) {
    wptr += wsize;

    if(wptr >= size)
        wptr = 0;

    wsize = 0;

    pthread_cond_signal(&data_available);
}
