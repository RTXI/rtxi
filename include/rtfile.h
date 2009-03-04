#include <pthread.h>
#include <string>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

class Fifo;

class RTFile {

public:

    RTFile(void);
    RTFile(const std::string &,int =O_CREAT|O_WRONLY|O_TRUNC,int =S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
    ~RTFile(void);

    int open(const std::string &,int =O_CREAT|O_WRONLY|O_TRUNC,int =S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
    void close(void);

    void write(void *,size_t);

private:

    static void *bounce(void *);
    void processData(void);

    bool done;

    int fd;
    std::string filename;

    pthread_t thread;

    Fifo *fifo;

}; // class RTFile
