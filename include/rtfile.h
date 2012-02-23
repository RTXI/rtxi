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

#include <pthread.h>
#include <string>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <fifo.h>

class RTFile {

public:

    RTFile(void);
    RTFile(const std::string &,int,mode_t =S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
    ~RTFile(void);

    bool open(const std::string &,int,mode_t =S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
    void close(void);

    size_t write(void *,size_t);
    size_t read(void *,size_t);

private:

    static void *bounce(void *);
    void processData(void);

    bool done;

    int fd;
    bool writing;

    pthread_t thread;
    Fifo fifo;

}; // class RTFile
