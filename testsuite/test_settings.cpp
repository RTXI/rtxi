#include <sstream>
#include <settings.h>
#include <stdlib.h>

#include "test_settings.h"

CPPUNIT_TEST_SUITE_REGISTRATION(TestSettings);

static const size_t VariableCount = 10;

static std::string random_string(void) {
    std::string str;

    for(size_t i=0;i<10;++i)
        str.append(1,26*(rand()/RAND_MAX)+97);

    return str;
}

void TestSettings::testSingleObject(void) {

    class TestSingleObject : virtual public Settings::Object {

    public:

        int _int[VariableCount];
        double _double[VariableCount];
        std::string _string[VariableCount];

        void doLoad(const Settings::Object::State &s) {
            for(size_t i=0;i<VariableCount;++i) {
                std::ostringstream str_i;
                str_i << i;
                _int[i] = s.loadInteger("int "+str_i.str());
                _double[i] = s.loadDouble("double "+str_i.str());
                _string[i] = s.loadString("string "+str_i.str());
            }
        }
        void doSave(Settings::Object::State &s) const {
            for(size_t i=0;i<VariableCount;++i) {
                std::ostringstream str_i;
                str_i << i;
                s.saveInteger("int "+str_i.str(),_int[i]);
                s.saveDouble("double "+str_i.str(),_double[i]);
                s.saveString("string "+str_i.str(),_string[i]);
            }
        }

    } object1, object2;

    for(size_t i=0;i<VariableCount;++i) {
        object1._int[i] = rand()-RAND_MAX/2;
        object1._double[i] = 1.0*(rand()-RAND_MAX/2);
        object1._string[i] = random_string();
    }

    object2.load(object1.save());

    for(size_t i=0;i<VariableCount;++i) {
        CPPUNIT_ASSERT_EQUAL(object1._int[i],object2._int[i]);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(object1._double[i],object2._double[i],abs(1e-5*object1._double[i]));
        CPPUNIT_ASSERT_EQUAL(object1._string[i],object2._string[i]);
    }
}

void TestSettings::testNestedObjects(void) {

    class TestNestedObjectsParent : virtual public Settings::Object {

    public:

        class TestNestedObjectsChild : virtual public Settings::Object {

        public:

            int _int[VariableCount];
            double _double[VariableCount];
            std::string _string[VariableCount];

            void doLoad(const Settings::Object::State &s) {
                for(size_t i=0;i<VariableCount;++i) {
                    std::ostringstream str_i;
                    str_i << i;
                    _int[i] = s.loadInteger("int "+str_i.str());
                    _double[i] = s.loadDouble("double "+str_i.str());
                    _string[i] = s.loadString("string "+str_i.str());
                }
            }
            void doSave(Settings::Object::State &s) const {
                for(size_t i=0;i<VariableCount;++i) {
                    std::ostringstream str_i;
                    str_i << i;
                    s.saveInteger("int "+str_i.str(),_int[i]);
                    s.saveDouble("double "+str_i.str(),_double[i]);
                    s.saveString("string "+str_i.str(),_string[i]);
                }
            }

        } nested_objects[VariableCount];

        void doLoad(const Settings::Object::State &s) {
            for(size_t i=0;i<VariableCount;++i) {
                std::ostringstream str;
                str << i;
                nested_objects[i].load(s.loadState(str.str()));
            }
        }
        void doSave(Settings::Object::State &s) const {
            for(size_t i=0;i<VariableCount;++i) {
                std::ostringstream str;
                str << i;
                s.saveState(str.str(),nested_objects[i].save());
            }
        }

    } object1, object2;

    for(size_t i=0;i<VariableCount;++i)
        for(size_t j=0;j<VariableCount;++j) {
            object1.nested_objects[i]._int[j] = rand()-RAND_MAX/2;
            object1.nested_objects[i]._double[j] = 1.0*(rand()-RAND_MAX/2);
            object1.nested_objects[i]._string[j] = random_string();
        }

    object2.load(object1.save());

    for(size_t i=0;i<VariableCount;++i)
        for(size_t j=0;j<VariableCount;++j) {
            CPPUNIT_ASSERT_EQUAL(object1.nested_objects[i]._int[j],object2.nested_objects[i]._int[j]);
            CPPUNIT_ASSERT_DOUBLES_EQUAL(object1.nested_objects[i]._double[j],object2.nested_objects[i]._double[j],abs(1e-5*object1.nested_objects[i]._double[j]));
            CPPUNIT_ASSERT_EQUAL(object1.nested_objects[i]._string[j],object2.nested_objects[i]._string[j]);
        }
}
