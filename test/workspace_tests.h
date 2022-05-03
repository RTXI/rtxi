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
public:
    std::string defaultWorkspaceName;
    std::string defaultInputChannelName = "CHANNEL INPUT";
    std::string defaultInputChannelDescription = "DEFAULT INPUT CHANNEL DESCRIPTION";
    std::string defaultOutputChannelName = "CHANNEL OUTPUT";
    std::string defaultOutputChannelDescription = "DEFAULT OUTPUT CHANNEL DESCRIPTION";
    std::string defaultParameterChannelName = "DEFAULT PARAMETER CHANNEL NAME";
    std::string defaultParameterChannelDescription = "DEFAULT PARAMETER CHANNEL DESCRIPTION";
    std::string defaultStateChannelName = "DEFAULT STATE CHANNEL NAME";
    std::string defaultStateChannelDescription = "DEFAULT STATE CHANNEL DESCRIPTION";
    std::string defaultEventChannelName = "DEFAULT EVENT CHANNEL NAME";
    std::string defaultEventChannelDescription = "DEFAULT EVENT CHANNEL DESCRIPTION";
    std::string defaultCommentChannelName = "DEFAULT COMMENT CHANNEL NAME";
    std::string defaultCommentChannelDescription = "DEFAULT COMMENT CHANNEL DESCRIPTION";
    IO::channel_t *defaultChannelList;

protected:
    WorkspaceInstanceTest() { 
        // Generates a default workspace with single input and output channel
        defaultWorkspaceName = "DEFAULT:INSTANCE:NAME";
        IO::channel_t defaultInputChannel_t = { 
            defaultInputChannelName,
            defaultInputChannelDescription,
            Workspace::INPUT
        };
        IO::channel_t defaultOutputChannel_t = {
            defaultOutputChannelName,
            defaultOutputChannelDescription,
            Workspace::OUTPUT
        };
        IO::channel_t defaultParameterChannel_t {
            defaultParameterChannelName,
            defaultParameterChannelDescription,
            Workspace::PARAMETER
        };
        IO::channel_t defaultStateChannel_t {
            defaultStateChannelName,
            defaultStateChannelDescription,
            Workspace::STATE
        };
        IO::channel_t defaultEventChannel_t {
            defaultEventChannelName,
            defaultEventChannelDescription,
            Workspace::EVENT
        };
        IO::channel_t defaultCommentChannel_t {
            defaultCommentChannelName,
            defaultCommentChannelDescription,
            Workspace::COMMENT
        };
        defaultChannelList = new IO::channel_t[6];
        defaultChannelList[0] = defaultInputChannel_t;
        defaultChannelList[1] = defaultOutputChannel_t;
        defaultChannelList[2] = defaultParameterChannel_t;
        defaultChannelList[3] = defaultStateChannel_t;
        defaultChannelList[4] = defaultEventChannel_t;
        defaultChannelList[5] = defaultCommentChannel_t;
        instance = new Workspace::Instance(defaultWorkspaceName, defaultChannelList, (size_t) 6);
    }
    ~WorkspaceInstanceTest() { }

    Workspace::Instance *instance;
    std::vector<IO::flags_t> flagsList = {Workspace::INPUT, Workspace::OUTPUT, Workspace::PARAMETER,
                                          Workspace::STATE, Workspace::EVENT, Workspace::COMMENT};
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
