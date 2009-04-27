#ifndef TEST_RT_OS_H
#define TEST_RT_OS_H

#include <cppunit/extensions/HelperMacros.h>

class TestRTOS : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(TestRTOS);

    CPPUNIT_TEST(testGetTime);

    CPPUNIT_TEST(testTaskCreate);
    CPPUNIT_TEST(testTaskPeriodic);
    CPPUNIT_TEST(testTaskIsRealtime);

    CPPUNIT_TEST_SUITE_END();

public:
  
    void setUp();
    void tearDown();

    void testGetTime();

    void testTaskCreate();
    void testTaskPeriodic();
    void testTaskIsRealtime();

};

#endif // TEST_RT_OS_H
