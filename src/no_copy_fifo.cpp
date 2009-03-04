#include <compiler.h>
#include <debug.h>
#include <no_copy_fifo.h>

#ifdef DEBUG
#define MAGIC 0x55
#endif

NoCopyFifo::NoCopyFifo(size_t s)
    : rptr(0), wptr(0), sem(0), rsize(0), wsize(0), size(s), wrap(s) {
#ifdef DEBUG
    data = new char[size+1];
    data[size] = MAGIC;
#else
    data = new char[size];
#endif
}

NoCopyFifo::~NoCopyFifo(void) {
#ifdef DEBUG
    if(data[size] != MAGIC)
        ERROR_MSG("NoCopyFifo::~NoCopyFifo : end of buffer exceeded\n");
#endif

    delete[] data;
}

void *NoCopyFifo::read(size_t n,bool blocking) {
#ifdef DEBUG
    if(unlikely(rsize))
        ERROR_MSG("NoCopyFifo::read : detected multiple concurrent reads\n");
#endif

    if(likely(wptr-rptr >= n || (wptr < rptr && wrap-rptr >= n))) {
        rsize = n;
        return data+rptr;
    }

#ifdef DEBUG
    if(unlikely(wptr < rptr && wrap-rptr < n))
        ERROR_MSG("NoCopyFifo::read : detected possible data misaligned\n");
#endif

    if(unlikely(blocking)) {
        while(!(wptr-rptr >= n || (wptr < rptr && wrap-rptr >= n)))
            sem.down();

        rsize = n;
        return data+rptr;
    }

    return 0;
}

void NoCopyFifo::readDone(void) {
    rptr += rsize;

#ifdef DEBUG
    if(unlikely(rptr > wrap))
        ERROR_MSG("NoCopyFifo::readDone : read beyond end of written buffer (rptr = %d, rsize = %d,wrap = %d)\n",rptr,rsize,wrap);
#endif

    if(unlikely(rptr >= wrap))
        rptr = 0;
    rsize = 0;
}

void *NoCopyFifo::write(size_t n) {
#ifdef DEBUG
    if(unlikely(wsize))
        ERROR_MSG("NoCopyFifo::write : detected multiple concurrent writes\n");
#endif

    if(likely((rptr > wptr && rptr-wptr >= n) || (wptr >= rptr && size-wptr >= n))) {
       wsize = n;
        return data+wptr;
    } else if(likely(wptr >= rptr && rptr > n)) {
        wrap = wptr;
        wptr = 0;
        wsize = n;
        return data+wptr;
    }

    DEBUG_MSG("NoCopyFifo::write : buffer full, data is being lost\n");
    return 0;
}

void NoCopyFifo::writeDone(void) {
    wptr += wsize;

#ifdef DEBUG
    if(unlikely(wptr > size))
        ERROR_MSG("NoCopyFifo::writeDone : write beyond end of allocated buffer\n");
#endif

    if(unlikely(wptr >= size))
        wptr = 0;
    wsize = 0;

    while(sem.value() < 0)
        sem.up();
    if(sem.value() == 0)
        sem.up();
}
