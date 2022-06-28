/*
         The Real-Time eXperiment Interface (RTXI)
         Copyright (C) 2011 Georgia Institute of Technology, University of Utah,
   Weill Cornell Medical College

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

#include <thread>

#include "event_tests.hpp"

TEST_F(EventObjectTest, EventProcessingWait)
{
  MockEventObject test_event;
  auto listener = [&test_event]() { test_event.done(); };
  std::thread test_thread(listener);
  test_event.wait();
  if (test_thread.joinable()) {
    test_thread.join();
  }
  ASSERT_EQ(true, test_event.isdone());
}

TEST_F(EventManagerTest, postEvent)
{
  auto event_manager = std::make_unique<Event::Manager>();
  MockEventObject event_obj;
  MockEventHandler event_handler;
  EXPECT_CALL(event_handler, receiveEvent(&event_obj));
  event_manager->registerHandler(&event_handler);
  event_manager->postEvent(&event_obj);
}

TEST_F(EventManagerTest, Registration)
{
  auto event_manager = std::make_unique<Event::Manager>();
  MockEventHandler event_handler;
  MockEventHandler event_handler_not_called;
  MockEventObject test_event;

  // handlers should be called an appropriate amount of times if registration
  // is working as it should
  EXPECT_CALL(event_handler, receiveEvent(&test_event)).Times(1);
  EXPECT_CALL(event_handler_not_called, receiveEvent(&test_event)).Times(0);

  // event manager shouldn't crash if it doesn't find the handler in
  // its registry
  event_manager->unregisterHandler(&event_handler);
  event_manager->registerHandler(&event_handler);
  event_manager->registerHandler(&event_handler);

  event_manager->postEvent(&test_event);
}
