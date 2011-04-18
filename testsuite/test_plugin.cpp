#include <plugin.h>

#include <cstdlib>

#include "test_plugin.h"

CPPUNIT_TEST_SUITE_REGISTRATION(TestPlugin);

void TestPlugin::setUp(void) {}

void TestPlugin::tearDown(void) {}

void TestPlugin::testLoadUnload(void) {
    std::string test_plugin_module;
    test_plugin_module += TESTSUITE_DIR;
    test_plugin_module += "/test_plugin_module.so";

    Plugin::Object *plugin = Plugin::Manager::getInstance()->load(test_plugin_module);

    CPPUNIT_ASSERT(plugin != NULL);
    CPPUNIT_ASSERT_EQUAL(test_plugin_module,plugin->getLibrary());

    Plugin::Manager::getInstance()->unload(plugin);
}
