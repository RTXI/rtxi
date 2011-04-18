#include <rt.h>

#include <cstdlib>

#include "test_rt_os.h"

CPPUNIT_TEST_SUITE_REGISTRATION(TestRTOS);

void TestRTOS::setUp(void) {
    RT::OS::initiate();
}

void TestRTOS::tearDown(void) {
    RT::OS::shutdown();
}

#define TARGET_SLEEP 100000ul

void TestRTOS::testGetTime(void) {
    long long time, prev_time;

    prev_time = RT::OS::getTime();

    for(size_t i=0;i<10;++i) {
        usleep(TARGET_SLEEP);
        time = RT::OS::getTime();
        CPPUNIT_ASSERT(time >= prev_time + TARGET_SLEEP*1e3);
        prev_time = time;
    }
}

static void *testTaskCreateHelper(void *arg) {
    bool *executed = reinterpret_cast<bool *>(arg);

    *executed = true;

    return NULL;
}

void TestRTOS::testTaskCreate(void) {
    RT::OS::Task task;
    bool executed = false;

    CPPUNIT_ASSERT_EQUAL(0,RT::OS::createTask(&task,testTaskCreateHelper,&executed,0));
    RT::OS::deleteTask(task);

    CPPUNIT_ASSERT(executed);
}

#define TARGET_PERIOD 10000000ll

struct test_task_periodic_t {
    RT::OS::Task task;
    long long avg_period;
};

static void *testTaskPeriodicHelper(void *arg) {
    test_task_periodic_t *args = reinterpret_cast<test_task_periodic_t *>(arg);
    long long time, prev_time;

    args->avg_period = 0;

    if(RT::OS::setPeriod(args->task,TARGET_PERIOD) != 0)
        return NULL;

    prev_time = RT::OS::getTime();
    for(size_t i=0;i<10;++i) {
        RT::OS::sleepTimestep(args->task);
        time = RT::OS::getTime();
        args->avg_period += (time-prev_time)/10;
        prev_time = time;
    }

    return NULL;
}

void TestRTOS::testTaskPeriodic(void) {
    test_task_periodic_t args;

    CPPUNIT_ASSERT_EQUAL(0,RT::OS::createTask(&args.task,testTaskPeriodicHelper,&args,0));
    RT::OS::deleteTask(args.task);

    CPPUNIT_ASSERT_DOUBLES_EQUAL(args.avg_period,TARGET_PERIOD,TARGET_PERIOD/10);
}

static void *testTaskIsRealtimeHelper(void *arg) {
    bool *isrealtime = reinterpret_cast<bool *>(arg);

    *isrealtime = RT::OS::isRealtime();

    return NULL;
}

void TestRTOS::testTaskIsRealtime(void) {
    RT::OS::Task task;
    bool isrealtime = false;

    CPPUNIT_ASSERT_EQUAL(0,RT::OS::createTask(&task,testTaskIsRealtimeHelper,&isrealtime,0));

    CPPUNIT_ASSERT(!RT::OS::isRealtime());

    RT::OS::deleteTask(task);
    CPPUNIT_ASSERT(isrealtime);
}
