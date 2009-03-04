#include <debug.h>
#include <fifo.h>
#include <rtfile.h>

RTFile::RTFile(void)
    : done(false), fd(-1), filename("") {
    fifo = new Fifo(1024*1024);
}

RTFile::RTFile(const std::string &name,int flags,int mode)
    : done(false), fd(-1), filename(name) {
    fifo = new Fifo(1024*1024);

    open(name,flags,mode);
}

RTFile::~RTFile(void) {
    close();
    delete fifo;
}

int RTFile::open(const std::string &name,int flags,int mode) {
    if(fd >= 0)
        close();

    fd = ::open(name.c_str(),flags,mode);
    if(fd >= 0) {
        filename = name;

        done = false;
        int err = pthread_create(&thread,0,RTFile::bounce,this);
        if(err)
            DEBUG_MSG("RTFile::open : pthread_create = %d\n",err);
    } else
        fd = -1;

    return fd;
}

void RTFile::close(void) {
    if(fd >= 0) {
        done = true;
        size_t zero = 0;
        fifo->write(&zero,sizeof(zero));
        pthread_join(thread,0);
        ::close(fd);
        fd = -1;
    }
}

void RTFile::write(void *data,size_t size) {
    if(!size)
        return;

    fifo->write(&size,sizeof(size));
    fifo->write(data,size);
}

void *RTFile::bounce(void *that) {
    reinterpret_cast<RTFile *>(that)->processData();
    return 0;
}

void RTFile::processData(void) {
    size_t size;
    while(fifo->read(&size,sizeof(size))) {
        if(size == 0 && done)
            break;

        char buffer[size];
        fifo->read(buffer,size);
        ::write(fd,buffer,size);
    }
}
