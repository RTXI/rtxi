#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/TestResult.h>
#include <cppunit/TestResultCollector.h>
#include <cppunit/TestRunner.h>
#include <cppunit/TestListener.h>
#include <cppunit/TextOutputter.h>

class TestProgressListener : public CppUnit::TestListener {

    void startTest(CppUnit::Test *test) {
        std::string name = test->getName();

        std::cout << "\trunning \"" << name.substr(name.find("::")+6) << "\"";
    };

    void addFailure(const CppUnit::TestFailure &) {
        std::cout << " Failed";
    };

    void endTest(CppUnit::Test *) {
        std::cout << "\n";
    };

    void startSuite(CppUnit::Test *test) {
        if(test->getName() == "All Tests")
            return;

        std::cout << "Testing the \"" << test->getName().substr(4) << "\" unit\n";
    };

    void endSuite(CppUnit::Test *test) {
        if(test->getName() == "All Tests") {
            std::cout << "==============================================\n\n";
            std::cout << "Summary:\n";
        } else
            std::cout << "\n";
    };

}; // class TestProgressListener

int main(void) {
    close(2);

    std::cout << "\n == RTXI Unit Testsuite == \n\n";

    CppUnit::TestResult controller;

    CppUnit::TestResultCollector result;
    controller.addListener(&result);

    TestProgressListener progress;
    controller.addListener(&progress);

    CppUnit::TestRunner runner;
    runner.addTest(CppUnit::TestFactoryRegistry::getRegistry().makeTest());

    runner.run(controller);

    CppUnit::TextOutputter outputter(&result,std::cout);
    outputter.write();

    return 0;
}
