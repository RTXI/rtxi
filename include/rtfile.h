#include <pthread.h>
#include <string>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <no_copy_fifo.h>

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
    NoCopyFifo fifo;

}; // class RTFile
