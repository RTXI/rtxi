/*
 * Copyright (C) 2004 Boston University
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License as
 *  published by the Free Software Foundation; either version 2 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <compiler.h>
#include <debug.h>
#include <fifo.h>
#include <string.h>

/**********************************************************************
 * Magic is a token added to the end of the fifo buffer used to check *
 *   if the end of the buffer has been over written.                  *
 **********************************************************************/

#ifdef DEBUG
#define MAGIC 0x55
#endif

#define AVAILABLE ((size+wptr-rptr)%size)
static inline size_t min(size_t x,size_t y,size_t z) {
    if(x < y) {
        if(x < z)
            return x;
        else
            return z;
    } else {
        if(y < z)
            return y;
        else
            return z;
    }
}

Fifo::Fifo(size_t s)
    : rptr(0), wptr(0), sem(0), size(s) {
#ifdef DEBUG
    data = new char[size+1];
    data[size] = MAGIC;
#else
    data = new char[size];
#endif
}

Fifo::~Fifo(void) {
#ifdef DEBUG
    if(data[size] != MAGIC)
        ERROR_MSG("Fifo::~Fifo : end of buffer exceeded\n");
#endif

    if(data) delete[] data;
}

size_t Fifo::read(void *buffer,size_t n,bool blocking) {
    size_t read = 0;

    if(likely(AVAILABLE >= n)) {
        if(n > size-rptr) {
            memcpy(buffer,reinterpret_cast<char *>(data)+rptr,size-rptr);
            memcpy(reinterpret_cast<char *>(buffer)+size-rptr,data,n-size+rptr);
        } else
            memcpy(buffer,data+rptr,n);
        read = n;
        rptr = (rptr+n)%size;
    } else if(unlikely(blocking))
        do {
            while(!AVAILABLE) sem.down();

            size_t chunk = min(AVAILABLE,n-read,size-rptr);
            memcpy(reinterpret_cast<char *>(buffer)+read,data+rptr,chunk);
            read += chunk;
            rptr = (rptr+chunk)%size;
        } while(read < n);

    return read;
}

size_t Fifo::write(const void *buffer,size_t n) {
    if(unlikely(n > size-AVAILABLE)) {
        ERROR_MSG("Fifo::write : fifo full, data lost\n");
        return 0;
    }

    if(n > size-wptr) {
        memcpy(data+wptr,buffer,size-wptr);
        memcpy(data,reinterpret_cast<const char *>(buffer)+(size-wptr),n-(size-wptr));
    } else
        memcpy(data+wptr,buffer,n);

    wptr = (wptr+n)%size;

    if(likely(sem.value() < 1))
        sem.up();

    return n;
}
