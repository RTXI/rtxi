#ifndef TEST_PLUGIN_H
#define TEST_PLUGIN_H

#include <cppunit/extensions/HelperMacros.h>

class TestPlugin : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(TestPlugin);

    CPPUNIT_TEST(testLoadUnload);

    CPPUNIT_TEST_SUITE_END();

public:
  
    void setUp();
    void tearDown();

    void testLoadUnload();

};

#endif // TEST_PLUGIN_H
