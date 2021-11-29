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

#ifndef WORKSPACE_TESTS_H
#define WORKSPACE_TESTS_H

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <workspace.h>

class WorkspaceInstanceTest : public ::testing::Test
{
protected:
    WorkspaceInstanceTest() { }
    ~WorkspaceInstanceTest() { }

    Workspace::Instance *instance;
};

class WorkspaceManagerTest : public ::testing::Test
{
protected:
    WorkspaceManagerTest() { }
    ~WorkspaceManagerTest() { }

    Workspace::Manager *manager;
};

class MockWorkspaceInstance : public Workspace::Instance
{
public:
    MOCK_METHOD(size_t, getCount, (IO::flags_t), (const));
    MOCK_METHOD(std::string, getName, (IO::flags_t, size_t), (const));
    MOCK_METHOD(std::string, getDescription, (IO::flags_t, size_t), (const));
    MOCK_METHOD(double, getValue, (IO::flags_t, size_t), (const));
    MOCK_METHOD(std::string, getValueString, (IO::flags_t, size_t), (const));
    MOCK_METHOD(void, setValue, (size_t, double), (const));
    MOCK_METHOD(void, setComment, (size_t, std::string), (const));
};
#endif
