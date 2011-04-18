#ifndef TEST_EVENT_H
#define TEST_EVENT_H

#include <cppunit/extensions/HelperMacros.h>

class TestEvent : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(TestEvent);

    CPPUNIT_TEST(testEvent);
    CPPUNIT_TEST(testEventRT);

    CPPUNIT_TEST_SUITE_END();

public:
  
    void setUp();
    void tearDown();

    void testEvent();
    void testEventRT();

};

#endif // TEST_EVENT_H
