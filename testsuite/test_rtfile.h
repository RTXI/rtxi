#ifndef TEST_RTFILE_H
#define TEST_RTFILE_H

#include <cppunit/extensions/HelperMacros.h>

class TestRTFile : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(TestRTFile);

    CPPUNIT_TEST(testSingleWrite);
    CPPUNIT_TEST(testLargeWrite);
    CPPUNIT_TEST(testSingleRead);
    CPPUNIT_TEST(testLargeRead);

    CPPUNIT_TEST_SUITE_END();

public:
  
    void setUp();
    void tearDown();

    void testSingleWrite();
    void testLargeWrite();
    void testSingleRead();
    void testLargeRead();

};

#endif // TEST_RTFILE_H
