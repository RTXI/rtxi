/*
         The Real-Time eXperiment Interface (RTXI)
         Copyright (C) 2011 Georgia Institute of Technology, University of Utah,
   Will Cornell Medical College

         This program is free software: you can redistribute it and/or modify
         it under the terms of the GNU General Public License as published by
         the Free Software Foundation, either version 3 of the License, or
         (at your option) any later version.

         This program is distributed in the hope that it will be useful,
         but WITHOUT ANY WARRANTY; without even the implied warranty of
         MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
         GNU General Public License for more details.

         You should have received a copy of the GNU General Public License
         along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

#include <typeinfo>

#include <settings_tests.h>

TEST_F(SettingsObjectTest, getID)
{
  object = new Settings::Object();
  Settings::Object* objectList = new Settings::Object[5];
  for (int i = 0; i < 5; ++i) {
    ASSERT_EQ(object->getID(), object->getID());
    ASSERT_EQ(objectList[i].getID(), objectList[i].getID());
    ASSERT_NE(object->getID(), objectList[i].getID());
  }
}

TEST_F(SettingsObjectTest, save)
{
  // the Save Object is defined inside the Settings class, and without it being
  // exposed to the user it doesn't make sense to test too much of it.
  // TODO: Perhaps move the save object to a public namespace? If so test and
  // document it
  object = new Settings::Object();
  auto saveObject = object->save();
  ASSERT_EQ(typeid(saveObject), typeid(Settings::Object::State));
}

TEST_F(SettingsObjectTest, load)
{
  // Unfortunately the Settings::Object class is tightly coupled with
  // Settings::Manager, which means that it is not possible (in my view) to test
  // these two classes individually without changing the classes.
  // TODO: Uncouple Settings::Object and Settings::Manager (remove friend
  // keyword, build messages, etc.)

  // // Define default values to save in settings state
  // double testdouble = 100.0;
  // std::string teststring = "teststring";
  // int testint = 100;

  // // Test the save and load of state information
  // object = new Settings::Object();
  // auto state = object->save();
  // state.saveDouble("testdouble", testdouble);
  // state.saveInteger("testinteger", testint);
  // state.saveString("teststring", teststring);
  // object->load(state);
  // auto retstate = object->save();
  // EXPECT_DOUBLE_EQ(retstate.loadDouble("testdouble"), testdouble);
  // EXPECT_EQ(retstate.loadInteger("testinteger"), testint);
  // EXPECT_EQ(retstate.loadString("string"), teststring);
}

TEST_F(SettingsObjectTest, deferred)
{
  // NOTE: See the load test for SettingsObjectTest
}

TEST_F(SettingsManagerTest, getInstance)
{
  manager = Settings::Manager::getInstance();
  ASSERT_EQ(manager, Settings::Manager::getInstance());
  ASSERT_EQ(manager, manager->getInstance());
}

TEST_F(SettingsManagerTest, getObject)
{
  manager = Settings::Manager::getInstance();
  // Objects are automatically registered to manager on creation
  MockSettingsObject* objectList = new MockSettingsObject[5];
  for (int i = 0; i < 5; ++i) {
    ASSERT_EQ(objectList[i].getID(),
              manager->getObject(objectList[i].getID())->getID());
  }
  delete[] objectList;
}

TEST_F(SettingsManagerTest, load)
{
  // TODO: Should test this with mock plugins
}

TEST_F(SettingsManagerTest, save)
{
  // TODO: should test this with mock plugins
}

TEST_F(SettingsManagerTest, foreachObject)
{
  // TODO: should test this with settings plugin
}
