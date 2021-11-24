/*
 	 The Real-Time eXperiment Interface (RTXI)
	 Copyright (C) 2011 Georgia Institute of Technology, University of Utah, Weill Cornell Medical College

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

#include <settings_tests.h>

TEST_F(SettingsObjectTest, getID)
{
    object = new Settings::Object();
    Settings::Object *objectList = new Settings::Object[5];
    for(int i = 0; i < 5; ++i)
    {
        ASSERT_EQ(object->getID(), object->getID());
        ASSERT_EQ(objectList[i].getID(), objectList[i].getID());
        ASSERT_NE(object->getID(), objectList[i].getID());
    }
}

TEST_F(SettingsObjectTest, save)
{
}

TEST_F(SettingsObjectTest, load)
{
}

TEST_F(SettingsObjectTest, deferred)
{
}

TEST_F(SettingsObjectTest, stateClassFuncs)
{
}

TEST_F(SettingsManagerTest, getInstance)
{
}

TEST_F(SettingsManagerTest, getObject)
{
}

TEST_F(SettingsManagerTest, load)
{
}

TEST_F(SettingsManagerTest, save)
{
}

TEST_F(SettingsManagerTest, foreachObject)
{
}

