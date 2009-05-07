#include <rt.h>

#include <cstdlib>

#include "test_rt.h"

CPPUNIT_TEST_SUITE_REGISTRATION(TestRT);

void TestRT::setUp(void) {
    RT::System::getInstance();
}

void TestRT::tearDown(void) {}

void TestRT::testPeriod(void) {
    long long target_period = 1000000ll;

    CPPUNIT_ASSERT(RT::System::getInstance()->setPeriod(target_period) == 0);
    CPPUNIT_ASSERT(RT::System::getInstance()->getPeriod() == target_period);
}

void TestRT::testThread(void) {
    struct TestThread : public RT::Thread {

        TestThread(void) 
            : count(0), avg_period(0ll) {
            pthread_cond_init(&done,NULL);
        };
        ~TestThread(void) {
            pthread_cond_destroy(&done);
        };

        void execute(void) {
            if(count++ == 0)
                prev_time = RT::OS::getTime();
            else if(count++ <= 10) {
                long long time = RT::OS::getTime();
                avg_period += (time-prev_time) / 10;
                prev_time = time;
            } else
                pthread_cond_signal(&done);
        };

        size_t count;
        long long prev_time;
        long long avg_period;
        pthread_cond_t done;

    } thread;

    long long target_period = 10000000ll;
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

    CPPUNIT_ASSERT(RT::System::getInstance()->setPeriod(target_period) == 0);

    thread.setActive(true);

    pthread_mutex_lock(&mutex);
    CPPUNIT_ASSERT(pthread_cond_wait(&thread.done,&mutex) == 0);
    pthread_mutex_unlock(&mutex);

    CPPUNIT_ASSERT(thread.avg_period <= 1.1*target_period && thread.avg_period >= -1.1*target_period);

    pthread_mutex_destroy(&mutex);
}

void TestRT::testDevice(void) {
    struct TestDevice : public RT::Device {

        TestDevice(void) 
            : count(0), avg_period(0ll), avg_exectime(0ll) {
            pthread_cond_init(&done,NULL);
        };
        ~TestDevice(void) {
            pthread_cond_destroy(&done);
        };

        void read(void) {
            read_time = RT::OS::getTime();
        };

        void write(void) {
            if(count++ == 0)
                prev_time = RT::OS::getTime();
            else if(count++ <= 10) {
                long long time = RT::OS::getTime();
                avg_period += (time-prev_time) / 10;
                avg_exectime += (time-read_time) / 10;
                prev_time = time;
            } else
                pthread_cond_signal(&done);
        };

        size_t count;
        long long prev_time;
        long long read_time;
        long long avg_period;
        long long avg_exectime;
        pthread_cond_t done;

    } device;

    long long target_period = 10000000ll;
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

    CPPUNIT_ASSERT(RT::System::getInstance()->setPeriod(target_period) == 0);

    device.setActive(true);

    pthread_mutex_lock(&mutex);
    CPPUNIT_ASSERT(pthread_cond_wait(&device.done,&mutex) == 0);
    pthread_mutex_unlock(&mutex);

    CPPUNIT_ASSERT(device.avg_exectime < target_period);
    CPPUNIT_ASSERT(device.avg_period <= 1.1*target_period && device.avg_period >= -1.1*target_period);

    pthread_mutex_destroy(&mutex);
}
