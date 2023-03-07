/*
         The Real-Time eXperiment Interface (RTXI)
         Copyright (C) 2011 Georgia Institute of Technology, University of Utah,
   Will Cornell Medical College

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

#include <sys/types.h>
#include <memory>

namespace RT::OS
{
/*!
Simple FIFO(First In First Out) for data transfer between RTXI threads

This data structure is a fundamental component to the inter-process
communication between threads spawned by RTXI. In particular this is used
for communication between the real-time thread and non-realtime(UI) threads.
This is platform and interface dependent, so the FIFO primitive used in
posix interface will be different than Xenomai's evl interface

This structure should not be used between two threads of the same priority.
It is meant only for realtime and non-realtime thread communications. In the
case where you need communication between two ui threads, consider using a
message queue. for communication between two RT threads, use the underlying
OS provided IPCs.
*/
class Fifo
{
public:
  Fifo() = default;  // default constructor
  Fifo(const Fifo& fifo) = delete;  // copy constructor
  Fifo& operator=(const Fifo& fifo) = delete;  // copy assignment operator
  Fifo(Fifo&&) = default;  // move constructor
  Fifo& operator=(Fifo&&) = default;  // move assignment operator
  virtual ~Fifo() = default;

  /*!
   * Read the data stored in the FIFO written by RT thread. Must be run
   * from non-rt thread.
   *
   * \param buf The buffer where the data from the buffer should be
   *     written to
   * \param data_size The size of the data to read from the buffer
   * \return n Number of elements read. Same as size.
   */
  virtual ssize_t read(void* buf, size_t data_size) = 0;

  /*!
   * Write to the FIFO storage for the RT thread. Must be run from non-rt
   * thread.
   *
   * \param buf The buffer holding the data to write to the FIFO.
   * \param data_size The size of the data to read from the buffer
   * \return n Number of elements written. Same as size.
   */
  virtual ssize_t write(void* buf, size_t data_size) = 0;

  /*!
   * Read the data stored in the FIFO written by non-RT thread. Must be run
   * from RT thread.
   *
   * \param buf The buffer where the data from the buffer should be
   *     written to
   * \param data_size The size of the data to read from the buffer
   * \return n Number of elements read. Same as size.
   */
  virtual ssize_t readRT(void* buf, size_t data_size) = 0;

  /*!
   * Write to the FIFO storage for the non-RT thread. Must be run from RT
   * thread.
   *
   * \param buf The buffer holding the data to write to the FIFO.
   * \param data_size The size of the data to read from the buffer
   * \return n Number of elements written. Same as size.
   */
  virtual ssize_t writeRT(void* buf, size_t data_size) = 0;

  /*!
   * Get the memory capacity of the fifo
   *
   * \returns The maximum amount of memory the fifo can hold
   */
  virtual size_t getCapacity() = 0;
};

/*!
 * Obtain the Fifo object for this architecture.
 *
 * The Object Returned is a Fifo object that holds an implementatin
 * specific fifo/pipe.
 *
 * \param fifo The fifo object to store the newly created fifo.
 * \param fifo_size The size of the fifo to create
 * \returns 0 if successful, and errno otherwise.
 */
int getFifo(std::unique_ptr<Fifo>& fifo, size_t fifo_size);
}  // namespace RT::OS

#endif /* FIFO_H */
