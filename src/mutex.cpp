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

#include <mutex.h>
#include <rt.h>

Mutex::Locker::Locker(Mutex *m)
    : mutex(m) {
    if(mutex)
        mutex->lock();
}

Mutex::Locker::~Locker(void) {
    if(mutex)
        mutex->unlock();
}

Mutex::Mutex(Mutex::type_t type) {
        pthread_mutexattr_t attr;
        pthread_mutexattr_init(&attr);

        switch(type) {
          case RECURSIVE:
              pthread_mutexattr_settype(&attr,PTHREAD_MUTEX_RECURSIVE_NP);
              break;
          default:
              pthread_mutexattr_settype(&attr,PTHREAD_MUTEX_FAST_NP);
        }

        pthread_mutex_init(&mutex,&attr);
        pthread_mutexattr_destroy(&attr);
}

Mutex::~Mutex(void) {
    pthread_mutex_destroy(&mutex);
}

void Mutex::lock(void) {
#ifdef DEBUG
    if(RT::OS::isRealtime()) {
        ERROR_MSG("Detected unsafe lock attempt in RT thread\n");
        PRINT_BACKTRACE();
        if(!tryLock())
            ERROR_MSG("Failed to obtain the lock\n");
        return;
    }
#endif // DEBUG

    pthread_mutex_lock(&mutex);
}

void Mutex::unlock(void) {
    pthread_mutex_unlock(&mutex);
}

bool Mutex::tryLock(void) {
    return pthread_mutex_trylock(&mutex) == 0;
}
