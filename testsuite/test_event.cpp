#include <event.h>

#include <cstdlib>

#include "test_event.h"

CPPUNIT_TEST_SUITE_REGISTRATION(TestEvent);

void TestEvent::setUp(void) {
    srand(time(NULL));
}

void TestEvent::tearDown(void) {}

static const char *TESTSUITE_TEST_EVENT = "test event";

static const char *param_name[] = {
    "param 1",
    "param 2",
    "param 3",
};

static const size_t num_params = sizeof(param_name)/sizeof(param_name[0]);

void TestEvent::testEvent(void) {
    class TestHandler : public Event::Handler {

    public:

        TestHandler(void) : called(false) {};
        virtual ~TestHandler(void) {};

        void receiveEvent(const Event::Object *e) {
            if(e->getName() == TESTSUITE_TEST_EVENT) {
                called = true;

                for(size_t i=0;i<num_params;++i)
                    args[i] = e->getParam(param_name[i]);
            }
        };

        bool called;
        void *args[num_params];

    } handler;

    Event::Object event(TESTSUITE_TEST_EVENT);

    for(size_t i=0;i<num_params;++i)
        event.setParam(param_name[i],reinterpret_cast<void *>(rand()));

    Event::Manager::getInstance()->postEvent(&event);

    CPPUNIT_ASSERT(handler.called);
    for(size_t i=0;i<num_params;++i)
        CPPUNIT_ASSERT_EQUAL(event.getParam(param_name[i]),handler.args[i]);
}

void TestEvent::testEventRT(void) {
    class TestHandler : public Event::RTHandler {

    public:

        TestHandler(void) : called(false) {};
        virtual ~TestHandler(void) {};

        void receiveEventRT(const Event::Object *e) {
            if(e->getName() == TESTSUITE_TEST_EVENT) {
                called = true;

                for(size_t i=0;i<num_params;++i)
                    args[i] = e->getParam(param_name[i]);
            }
        };

        bool called;
        void *args[num_params];

    } handler;
    

    Event::Object event(TESTSUITE_TEST_EVENT);

    for(size_t i=0;i<num_params;++i)
        event.setParam(param_name[i],reinterpret_cast<void *>(rand()));

    Event::Manager::getInstance()->postEventRT(&event);

    CPPUNIT_ASSERT(handler.called);
    for(size_t i=0;i<num_params;++i)
        CPPUNIT_ASSERT_EQUAL(event.getParam(param_name[i]),handler.args[i]);
}
