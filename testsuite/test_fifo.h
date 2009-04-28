#ifndef TEST_FIFO_H
#define TEST_FIFO_H

#include <cppunit/extensions/HelperMacros.h>

class Fifo;

class TestFifo : public CppUnit::TestCase {
    CPPUNIT_TEST_SUITE(TestFifo);

    CPPUNIT_TEST(testSingleReadWrite);
    CPPUNIT_TEST(testWrappingReadWrite);

    CPPUNIT_TEST_SUITE_END();

public:

    void *operator new[](size_t);
    void operator delete[](void *);

    void testSingleReadWrite();
    void testWrappingReadWrite();

};

#endif // TEST_FIFO_H
