/*
	 The Real-Time eXperiment Interface (RTXI)
	 Copyright (C) 2011 Georgia Institute of Technology, University of Utah, Weill Cornell Medical College

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

#include <debug.h>
#include <fifo.h>
#include <string.h>

#define AVAILABLE    ((size+wptr-rptr)%size)

Fifo::Fifo(size_t s):rptr(0), wptr(0), size(s)
{
    data = new char[size];

    pthread_mutex_init(&mutex,NULL);
    pthread_cond_init(&data_available,NULL);
}

Fifo::~Fifo(void)
{
    if (data) delete[] data;

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&data_available);
}

size_t Fifo::read(void *buffer,size_t n,bool blocking)
{
    // Acquire the data lock
    if (blocking)
        pthread_mutex_lock(&mutex);
    else if (pthread_mutex_trylock(&mutex) != 0)
        return 0;

    // Check that enough data is available
    if (AVAILABLE < n)
        if (blocking)
            {
                do
                    {
                        pthread_cond_wait(&data_available,&mutex);
                    }
                while (AVAILABLE < n);
            }
        else
            {
                pthread_mutex_unlock(&mutex);
                return 0;
            }

    // Copy the data from the fifo
    if (size-rptr < n)
        {
            size_t m = size-rptr;
            memcpy(buffer,data+rptr,m);
            memcpy(reinterpret_cast<char *>(buffer)+m,data,n-m);
        }
    else
        memcpy(buffer,data+rptr,n);
    rptr = (rptr+n)%size;

    pthread_mutex_unlock(&mutex);
    return n;
}

size_t Fifo::write(const void *buffer,size_t n)
{
    if (n >= size-AVAILABLE)
        {
            ERROR_MSG("Fifo::write : fifo full, data lost\n");
            return 0;
        }

    if (n > size-wptr)
        {
            size_t m = size-wptr;
            memcpy(data+wptr,buffer,m);
            memcpy(data,reinterpret_cast<const char *>(buffer)+m,n-m);
        }
    else
        memcpy(data+wptr,buffer,n);
    wptr = (wptr+n)%size;

    pthread_cond_signal(&data_available);

    return n;
}
