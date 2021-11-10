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

#ifndef EVENT_MANAGER_TESTS_H
#define EVENT_MANAGER_TESTS_H

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <rt.h>
#include <event.h>

class EventManagerTest : public ::testing::Test
{
protected:
    EventManagerTest() { }
    ~EventManagerTest() { }

    Event::Manager *event_manager;
    RT::System *system;
};

class MockEventObject : public Event::Object
{
public:
    MockEventObject(const char * n) : Event::Object(n) { }
    ~MockEventObject() { }

    MOCK_METHOD(void *, getParam, (const char *), (const));
    MOCK_METHOD(void, setParam, (const char *, void *), (const));
    MOCK_METHOD(const char *, getName, (), (const));
};

class MockEventHandler : public Event::Handler
{
public:
    MOCK_METHOD(void, receiveEvent, (const Event::Object *), (override));
};

class MockEventRTHandler : public Event::RTHandler
{
public:
    MOCK_METHOD(void, receiveEventRT, (const Event::Object *), (override));
};

#endif
