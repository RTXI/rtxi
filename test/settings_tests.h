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

#ifndef SETTINGS_TESTS_H
#define SETTINGS_tESTS_H

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <settings.h>

class SettingsManagerTest : public ::testing::Test
{
protected:
    SettingsManagerTest() { }
    ~SettingsManagerTest() { }

    Settings::Manager *manager;
};

class SettingsObjectTest : public ::testing::Test
{
protected:
    SettingsObjectTest() { }
    ~SettingsObjectTest() { }

    Settings::Object *object;
};

class MockSettingsObject : public Settings::Object
{
public:
    //MOCK_METHOD(Settings::Object::ID, getID, (), (const));
    MOCK_METHOD(Settings::Object::State, save, (), (const));
    MOCK_METHOD(void, load, (const State &), (const));
    MOCK_METHOD(void, deferred, (const State &), (const));
};
#endif
