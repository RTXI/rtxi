#include <rt.h>
#include <sem.h>
#include <unistd.h>

Semaphore::Semaphore(size_t n)
    : count(n) {}

Semaphore::~Semaphore(void) {}

void Semaphore::down(void) {
#ifdef DEBUG
    if(RT::OS::isRealtime()) {
        ERROR_MSG("Detected unsafe down attempt in RT thread\n");
        PRINT_BACKTRACE();
        return;
    }
#endif // DEBUG

    --count;
    while(count < 0) usleep(1000);
}

void Semaphore::up(void) {
    ++count;
}

int Semaphore::value(void) {
    return count;
}
