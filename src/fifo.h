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

#ifndef FIFO_H
#define FIFO_H

#include <cstdlib>
#include <pthread.h>

/*!
Simple FIFO(First In First Out) for data transfer between components in RTXI

\sa AtomicFifo
*/
class Fifo
{

public:

    /*!
    * FIFO constructor. Builds a simple FIFO object.
    *
    * \param s Size of the FIFO 
    */
    Fifo(size_t);

    /*!
    * FIFO Destructor
    */
    ~Fifo(void);

    /*!
    * Read the data stored in the FIFO. clears out the data after
    * the operation is complete.
    * 
    * \param buffer The buffer where the data from the buffer should be
    *     written to
    * \param size The size of the data to read from the buffer
    * \param blocking Whether the thread should expect to be blocked or not
    * \return n Number of elements read. Same as size.
    */
    size_t read(void *,size_t,bool =true);

    /*!
    * Write to the FIFO. 
    * 
    * \param buffer The buffer holding the data to write to the FIFO. 
    * \param size The size of the data to read from the buffer
    * \return n Number of elements written. Same as size.
    */
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
