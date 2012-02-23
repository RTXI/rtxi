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
#include <fifo.h>
#include <rtfile.h>
#include <string.h>
#include <cstdlib>

RTFile::RTFile(void)
    : done(true), fd(-1), writing(false), fifo(1024*1024) {}

RTFile::RTFile(const std::string &name,int flags,mode_t mode)
    : done(true), fd(-1), writing(false), fifo(1024*1024) {
    open(name,flags,mode);
}

RTFile::~RTFile(void) {
    close();
}

bool RTFile::open(const std::string &name,int flags,mode_t mode) {
    if(fd >= 0)
        close();

    // Don't open a file for both reading and writing
    if(flags & O_RDWR)
        return false;

    fd = ::open(name.c_str(),flags,mode);
    if(fd < 0)
        return false;

    done = false;
    writing = (flags & O_WRONLY) ? true : false;

    if(pthread_create(&thread,0,RTFile::bounce,this) != 0) {
        ::close(fd);
        fd = -1;
        return false;
    }

    return true;
}

void RTFile::close(void) {
    if(fd >= 0) {
        done = true;

        if(writing) {
            size_t zero = 0;
            fifo.write(&zero,sizeof(zero));
        } else
            pthread_cancel(thread);
        pthread_join(thread,0);
        ::fsync(fd);
        ::close(fd);
        fd = -1;
    }
}

size_t RTFile::write(void *data,size_t size) {
    if(!size)
        return 0;

    fifo.write(&size,sizeof(size));
    fifo.write(data,size);

    return size;
}

size_t RTFile::read(void *data,size_t size) {
    if(!size)
        return 0;

    fifo.read(data,size);

    return size;
}

void *RTFile::bounce(void *that) {
    reinterpret_cast<RTFile *>(that)->processData();
    return 0;
}

template<typename T>
static T MIN(T a,T b) {
    if(a < b)
        return a;
    else
        return b;
}

static size_t bytes_remaining(int fd) {
    struct stat fstats;
    off_t fpos;

    fpos = lseek(fd,0,SEEK_CUR);
    fstat(fd,&fstats);

    return fstats.st_size-fpos;
}

void RTFile::processData(void) {
    if(writing) {
        size_t size;
        char buffer[1024];

        while(!done) {
            fifo.read(&size,sizeof(size));

            if(size == 0)
                return;

            size_t nwrite = 0;
            while(nwrite < size) {
                size_t chunk = size < 1024 ? size : 1024;

                fifo.read(buffer,chunk);
                nwrite += ::write(fd,buffer,chunk);
            }
        }

        // write any remaining data
        fifo.read(&size,sizeof(size));
        while(size > 0) {
            size_t nwrite = 0;
            while(nwrite < size) {
                size_t chunk = size < 1024 ? size : 1024;

                fifo.read(buffer,chunk);
                nwrite += ::write(fd,buffer+nwrite,size-nwrite);
            }

            fifo.read(&size,sizeof(size));
        }
    } else {
        size_t size;
        char buffer[1024];

        while(!done) {
            size = MIN(static_cast<size_t>(1024),bytes_remaining(fd));

            if(size == 0) {
                sleep(1);
                continue;
            }

            size_t nread = 0;
            while(nread < size)
                nread += ::read(fd,buffer+nread,size-nread);

            fifo.write(buffer,size);
        }
    }
}
