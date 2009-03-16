
#include <unistd.h>
#include <sys/wait.h>
#include <debug.h>
#include <cmdline.h>
#include <compiler.h>
#include <mutex.h>
#include <stdlib.h>
#include <string.h>

CmdLine::CmdLine(void) {

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
    if(unlikely(!instance))
        initialize();

    size_t size = strlen(cmd.c_str())+1;

    write(instance->fds[1],&size,sizeof(size));
    write(instance->fds[1],cmd.c_str(),size);
     
    int retval;
    read(instance->fdm[0],&retval,sizeof(retval));

    return retval;
}

static Mutex mutex;
CmdLine *CmdLine::instance = 0;

void CmdLine::initialize(void) {
    Mutex::Locker lock(&::mutex);
    if(!instance) {
        static CmdLine cmdline;
        instance = &cmdline;
    }
}
