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

#ifndef MUTEX_H
#define MUTEX_H

#include <pthread.h>

/*!
 *
 */
class Mutex {

public:

    /*!
     *
     */
    class Locker {

    public:

        /*!
         *
         */
        Locker(Mutex *m);
        ~Locker(void);

    private:

        Mutex *mutex;

    };

    /*!
     *
     */
    enum type_t {
        FAST,       /*!< */
        RECURSIVE,  /*!< */
    };

    /*!
     *
     */
    Mutex(type_t type =FAST);
    ~Mutex(void);

    /*!
     *
     */
    void lock(void);

    /*!
     *
     */
    void unlock(void);

    /*!
     *
     */
    bool tryLock(void);

private:

    pthread_mutex_t mutex;

}; // class Mutex

#endif // MUTEX_H
