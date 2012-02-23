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

#include <unistd.h>
#include <sys/wait.h>
#include <debug.h>
#include <cmdline.h>
#include <stdlib.h>
#include <string.h>

CmdLine::CmdLine(void)
     : done(false), mutex(Mutex::RECURSIVE) {
     
     pipe(fdm);
     pipe(fds);

     if(!(child = fork())) {
	  size_t size;
	  
	  read(fds[0],&size,sizeof(size));
	  while(size) {
	       int retval;
	       char cmd[size];
	       
	       read(fds[0],cmd,size);
	       DEBUG_MSG("executing : \"%s\"\n",cmd);
	       retval = system(cmd);
	       
	       write(fdm[1],&retval,sizeof(retval));
	       
	       read(fds[0],&size,sizeof(size));
	  }
	  
	  _exit(0);
     }
}

CmdLine::~CmdLine(void) {
    int zero = 0;
    write(fds[1],&zero,sizeof(zero));

    waitpid(child,0,0);
}

int CmdLine::execute(const std::string &cmd)
{
     size_t size = strlen(cmd.c_str())+1;

     write(fds[1],&size,sizeof(size));
     write(fds[1],cmd.c_str(),size);
     
     int retval;
     read(fdm[0],&retval,sizeof(retval));
     
     return retval;
}

static Mutex mutex;
CmdLine *CmdLine::instance = 0;

CmdLine *CmdLine::getInstance(void) {
    if(instance)
        return instance;

    Mutex::Locker lock(&::mutex);
    if(!instance) {
        static CmdLine cmdline;
        instance = &cmdline;
    }

    return instance;
}
