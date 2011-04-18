#ifndef SEM_H
#define SEM_H

#include <unistd.h>

class Semaphore {

public:

    Semaphore(size_t n =0);
    ~Semaphore(void);

    void down(void);
    void up(void);
    int value(void);

private:

    int count;

}; // class Semaphore

#endif // SEM_H
