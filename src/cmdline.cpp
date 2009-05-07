
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
