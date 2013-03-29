#include <test_plugin_module.h>

extern "C" Plugin::Object *createRTXIPlugin(void) {
    return new TestPluginModule();
}

TestPluginModule::TestPluginModule(void) {}

TestPluginModule::~TestPluginModule(void) {}
