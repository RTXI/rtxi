#ifndef TEST_SETTINGS_H
#define TEST_SETTINGS_H

#include <cppunit/extensions/HelperMacros.h>

class TestSettings : public CppUnit::TestCase {
    CPPUNIT_TEST_SUITE(TestSettings);

    CPPUNIT_TEST(testSingleObject);
    CPPUNIT_TEST(testNestedObjects);

    CPPUNIT_TEST_SUITE_END();

public:

    void testSingleObject(void);
    void testNestedObjects(void);

};

#endif // TEST_SETTINGS_H
