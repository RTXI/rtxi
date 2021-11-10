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

#include <eventmanager_tests.h>
#include <gmock/gmock.h>
#include <event.h>

TEST_F(EventManagerTest, getInstance)
{
    event_manager = Event::Manager::getInstance();
    EXPECT_EQ(event_manager, Event::Manager::getInstance());
    EXPECT_EQ(event_manager, event_manager->getInstance());
}

TEST_F(EventManagerTest, postEvent)
{
    event_manager = Event::Manager::getInstance();
    const char *TEST_EVENT_NAME = "EventManagerTest - postEvent : Test Event";
    MockEventObject event_object(TEST_EVENT_NAME);
    MockEventHandler event_handler;
    MockEventRTHandler event_rthandler;
    EXPECT_CALL(event_handler, receiveEvent).Times(::testing::AtLeast(1));
    EXPECT_CALL(event_rthandler, receiveEventRT).Times(::testing::Exactly(0));
    event_manager->postEvent(&event_object);
}

TEST_F(EventManagerTest, postEventRT)
{
    event_manager = Event::Manager::getInstance();
    const char *TEST_EVENT_NAME = "EventManagerTest - postEventRT : Test Event";
    MockEventObject event_object(TEST_EVENT_NAME);
    MockEventRTHandler event_rthandler;
    MockEventHandler event_handler;
    EXPECT_CALL(event_rthandler, receiveEventRT).Times(::testing::AtLeast(1));
    EXPECT_CALL(event_handler, receiveEvent).Times(::testing::Exactly(0));
    event_manager->postEventRT(&event_object);
}


