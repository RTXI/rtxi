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

#include <rt.h>
#include <rwlock.h>

RWLock::ReadLocker::ReadLocker(RWLock *rw)
    : rwlock(rw) {
    if(rwlock)
        rwlock->readLock();
}

RWLock::ReadLocker::~ReadLocker(void) {
    if(rwlock)
        rwlock->unlock();
}

RWLock::WriteLocker::WriteLocker(RWLock *rw)
    : rwlock(rw) {
    if(rwlock)
        rwlock->writeLock();
}

RWLock::WriteLocker::~WriteLocker(void) {
    if(rwlock)
        rwlock->unlock();
}

RWLock::RWLock(RWLock::type_t type) {
    pthread_rwlockattr_t attr;
    pthread_rwlockattr_init(&attr);

    switch(type) {
      case PREFER_READER:
          pthread_rwlockattr_setkind_np(&attr,PTHREAD_RWLOCK_PREFER_READER_NP);
          break;
      case PREFER_WRITER_NONRECURSIVE:
          pthread_rwlockattr_setkind_np(&attr,PTHREAD_RWLOCK_PREFER_WRITER_NONRECURSIVE_NP);
          break;
      default:
          pthread_rwlockattr_setkind_np(&attr,PTHREAD_RWLOCK_PREFER_WRITER_NP);
    }

    pthread_rwlock_init(&rwlock,&attr);
    pthread_rwlockattr_destroy(&attr);
}

RWLock::~RWLock(void) {
    pthread_rwlock_destroy(&rwlock);
}

void RWLock::readLock(void) {
#ifdef DEBUG
    if(RT::OS::isRealtime()) {
        ERROR_MSG("Detected unsafe lock attempt in RT thread\n");
        PRINT_BACKTRACE();
        if(!tryReadLock())
            ERROR_MSG("Failed to obtain the lock\n");
        return;
    }
#endif // DEBUG

    pthread_rwlock_rdlock(&rwlock);
}

void RWLock::writeLock(void) {
#ifdef DEBUG
    if(RT::OS::isRealtime()) {
        ERROR_MSG("Detected unsafe lock attempt in RT thread\n");
        PRINT_BACKTRACE();
        if(!tryWriteLock())
            ERROR_MSG("Failed to obtain the lock\n");
        return;
    }
#endif // DEBUG

    pthread_rwlock_wrlock(&rwlock);
}

void RWLock::unlock(void) {
    pthread_rwlock_unlock(&rwlock);
}

bool RWLock::tryReadLock(void) {
    return pthread_rwlock_tryrdlock(&rwlock) == 0;
}

bool RWLock::tryWriteLock(void) {
    return pthread_rwlock_trywrlock(&rwlock) == 0;
}
