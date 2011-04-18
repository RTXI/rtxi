/*
 * Copyright (C) 2005 Boston University
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
