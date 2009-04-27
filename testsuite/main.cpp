#include <cppunit/TextOutputter.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>

int main(void) {
    CppUnit::TextUi::TestRunner runner;

    runner.setOutputter(new CppUnit::TextOutputter(&runner.result(),std::cout));
    runner.addTest(CppUnit::TestFactoryRegistry::getRegistry().makeTest());
    
    return runner.run() ? 0 : 1;
}
