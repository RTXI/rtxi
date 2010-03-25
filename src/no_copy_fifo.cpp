#include <debug.h>
#include <no_copy_fifo.h>

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

    void *ptr = NULL;

    for(;;) {
        if(rptr == wrap)
            rptr = 0;

        if(rptr <= wptr) {
            if(wptr-rptr >= n) {
                rsize = n;
                ptr = data+rptr;
                break;
            } else {
                if(blocking)
                    pthread_cond_wait(&data_available,&mutex);
                else {
                    ptr = NULL;
                    break;
                }
            }
        } else { // rptr > wptr
            if(wrap-rptr >= n) {
                rsize = n;
                ptr = data+rptr;
                break;
            } else {
                ERROR_MSG("NoCopyFifo::read : rptr > wptr && wrap-rptr < n : this shouldn't happen, likely error by fifo user\n");
                ptr = NULL;
                break;
            }
        }
    };

    pthread_mutex_unlock(&mutex);
    return ptr;
}

void NoCopyFifo::readDone(void) {
    rptr += rsize;

    if(rptr == wrap)
        rptr = 0;
    else if(rptr > wrap) {
        ERROR_MSG("NoCopyFifo::readDone : detected read beyond written data\n");
        rptr = 0;
    }

    rsize = 0;
}

void *NoCopyFifo::write(size_t n) {
    void *ptr;

    if(rptr <= wptr) {
        if(size-wptr >= n) {
            wsize = n;
            ptr = data+wptr;
        } else if(rptr > n) {
            wrap = wptr;
            wptr = 0;

            wsize = n;
            ptr = data+wptr;
        } else {
            pthread_cond_signal(&data_available);
            ptr = NULL;
        }
    } else { // rptr > wptr
        if(rptr-wptr-1 >= n) {
            wsize = n;
            ptr = data+wptr;
        } else {
            pthread_cond_signal(&data_available);
            ptr = NULL;
        }
    }

    return ptr;
}

void NoCopyFifo::writeDone(void) {
    wptr += wsize;

    if(wptr == size)
        wptr = 0;
    else if(wptr > size) {
        ERROR_MSG("NoCopyFifo::writeDone : detected write beyond buffer boundary\n");
        wptr = 0;
    }

    wsize = 0;

    pthread_cond_signal(&data_available);
}
