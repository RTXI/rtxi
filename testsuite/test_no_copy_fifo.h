#ifndef TEST_NO_COPY_FIFO_H
#define TEST_NO_COPY_FIFO_H

#include <cppunit/extensions/HelperMacros.h>

class TestNoCopyFifo : public CppUnit::TestCase {
    CPPUNIT_TEST_SUITE(TestNoCopyFifo);

    CPPUNIT_TEST(testSingleReadWrite);
    CPPUNIT_TEST(testWrappingReadWrite);

    CPPUNIT_TEST_SUITE_END();

public:

    void testSingleReadWrite();
    void testWrappingReadWrite();

};

#endif // TEST_NO_COPY_FIFO_H
