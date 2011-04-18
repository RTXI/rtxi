#ifndef TEST_RT_H
#define TEST_RT_H

#include <cppunit/extensions/HelperMacros.h>

class TestRT : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(TestRT);

    CPPUNIT_TEST(testPeriod);
    CPPUNIT_TEST(testThread);
    CPPUNIT_TEST(testDevice);

    CPPUNIT_TEST_SUITE_END();

public:
  
    void setUp();
    void tearDown();

    void testPeriod();
    void testThread();
    void testDevice();

};

#endif // TEST_RT_H
