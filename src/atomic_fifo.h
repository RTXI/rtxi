/*
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

#ifndef ATOMIC_FIFO_H
#define ATOMIC_FIFO_H

#include <cstdlib>
#include <atomic>

//! Lockfree SINGLE producer / SINGLE consumer FIFO
/*
 * Uses C++11 standard atomic library to implement a lockfree FIFO.
 * It is an absolute requirement only one thread produces ( calls write() )
 * and only one thread consumes ( calls read() ).
 *
 */

class AtomicFifo
{

public:
    AtomicFifo(size_t);
    ~AtomicFifo(void);

    /*!
     * Function for writing data to atomic FIFO
     *
     * \param buffer Memory destination
     * \param itemSize Size of memory chunk to be copied
     */
    bool write(const void *buffer, size_t itemSize);

    /*!
     * Function for reading data from atomic FIFO
     *
     * \param buffer Memory source
     * \param itemSize Size of memory chunk to be copied into fifo
     */
    bool read(void *buffer, size_t itemSize);

    /*!
     *
     * Function to check if FIFO is truly atomic for the hardware architecture
     *
     */
    bool isLockFree() const;
private:
    size_t increment(size_t current_ptr, size_t itemSize) const;

    char *data;
    std::atomic<size_t> head;
    std::atomic<size_t> tail;
    size_t fifoSize;

};

#endif /* ATOMIC_FIFO_H */
