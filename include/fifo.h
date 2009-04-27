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

#ifndef FIFO_H
#define FIFO_H

#include <cstdlib>
#include <pthread.h>

class Fifo
{

public:

    Fifo(size_t);
    ~Fifo(void);

    size_t read(void *,size_t,bool =true);
    size_t write(const void *,size_t);

private:

    char *data;
    volatile int rptr;
    volatile int wptr;
    size_t size;
    pthread_mutex_t mutex;
    pthread_cond_t data_available;

};

#endif /* FIFO_H */
