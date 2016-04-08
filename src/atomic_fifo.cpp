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

#include <debug.h>
#include <iostream>
#include <atomic_fifo.h>
#include <string.h>

AtomicFifo::AtomicFifo(size_t s)
    : head(0), tail(0), fifoSize(s)
{
    data = new char[fifoSize];
}

AtomicFifo::~AtomicFifo(void)
{
    if(data) delete[] data;
}

bool AtomicFifo::write(const void *buffer,size_t itemSize)   // It is an absolute requirement only one thread calls write
{
    // Uses sequential consistent memory order, all atomic operations are
    // guaranteed not to be reordered within the same thread
    const auto current_tail = tail.load(std::memory_order_seq_cst);

    if( itemSize >= fifoSize - ((fifoSize + current_tail - head.load(std::memory_order_seq_cst)) % fifoSize) )
        {
            ERROR_MSG("AtomicFifo::write : fifo full, data lost\n");
            return false;
        }

    if(itemSize > fifoSize - current_tail)   // If needed, data is split between end and beginning of fifo
        {
            size_t m = fifoSize - current_tail;
            memcpy(data + current_tail, buffer, m); // Copy first part of data
            memcpy(data, reinterpret_cast<const char *>(buffer) + m, itemSize - m); // Copy last part of data
        }
    else
        memcpy(data + current_tail, buffer, itemSize);

    tail.store(increment(current_tail, itemSize));

    return true;
}

bool AtomicFifo::read(void *buffer,size_t itemSize)   // It is an absolute requirement only one thread calls read
{
    // Uses strictest memory order, atomic operations are guaranteed not to be
    // reordered within the same thread
    const auto current_head = head.load(std::memory_order_seq_cst);

    // Check if there is data to be read, and it is at least as large as item size
    // Left side of logical returns number of bytes available to be read
    if( ((fifoSize + tail.load(std::memory_order_seq_cst) - current_head) % fifoSize) < itemSize )
        {
            return false; // no data to be read
        }

    // Copy data from fifo to buffer
    if(fifoSize - current_head < itemSize)   // If data is split between end and beginning of fifo
        {
            size_t m = fifoSize - current_head;
            memcpy(buffer, data + current_head, m); // Retrieve first part of data
            memcpy(reinterpret_cast<char *>(buffer) + m, data, itemSize - m ); // Retrieve last part of data
        }
    else
        memcpy(buffer, data + current_head, itemSize);

    head.store(increment(current_head, itemSize));

    return true;
}

size_t AtomicFifo::increment(size_t current_ptr, size_t itemSize) const
{
    return (current_ptr + itemSize) % fifoSize;
}

bool AtomicFifo::isLockFree() const
{
    return (tail.is_lock_free() && head.is_lock_free());
}
