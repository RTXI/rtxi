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

#ifndef RW_LOCK_H
#define RW_LOCK_H

#include <pthread.h>

class RWLock {

public:

    class ReadLocker {

    public:

        ReadLocker(RWLock *);
        ~ReadLocker(void);

    private:

        RWLock *rwlock;

    }; // class ReadLocker

    class WriteLocker {

    public:

        WriteLocker(RWLock *);
        ~WriteLocker(void);

    private:

        RWLock *rwlock;

    }; // class WriteLocker

    enum type_t {
        PREFER_READER,
        PREFER_WRITER,
        PREFER_WRITER_NONRECURSIVE,
    };

    RWLock(type_t =PREFER_WRITER);
    ~RWLock(void);

    void readLock(void);
    void writeLock(void);

    void unlock(void);

    bool tryReadLock(void);
    bool tryWriteLock(void);

private:

    pthread_rwlock_t rwlock;

}; // class RWLock

#endif // RW_LOCK_H
