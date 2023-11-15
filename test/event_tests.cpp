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

#include <thread>

#include "event_tests.hpp"

TEST_F(EventObjectTest, ParameterTests)
{
  const std::string TEST_EVENT_PARAM = "TEST_PARAM";
  const bool TEST_EVENT_PARAM_VALUE = true;
  Event::Object event(Event::Type::NOOP);
  event.setParam(TEST_EVENT_PARAM, std::any(TEST_EVENT_PARAM_VALUE));
  ASSERT_EQ(event.getName(), Event::type_to_string(Event::Type::NOOP));
  ASSERT_EQ(std::any_cast<bool>(event.getParam(TEST_EVENT_PARAM)),
            TEST_EVENT_PARAM_VALUE);
  ASSERT_FALSE(event.getParam("DOESNOTEXIST").has_value());
}

TEST_F(EventObjectTest, EventProcessingWait)
{
  Event::Object test_event(Event::Type::NOOP);
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
  Event::Object event_obj(Event::Type::NOOP);
  MockEventHandler event_handler;
  EXPECT_CALL(event_handler, receiveEvent(&event_obj));
  event_manager->registerHandler(&event_handler);
  event_manager->postEvent(&event_obj);
  event_obj.wait();

  // before exiting the test we must unregister event handler
  // because the event thread may call event handler functions
  // after the handler has been destroyed from stack
  event_manager->unregisterHandler(&event_handler);
}

// TODO: Event registration test seems to fail randomly. Seems to be caused
// by gtest inability to handle multiple threads. Should replace EXPECT_CALL
// with a more reliable mechanism to check expected calls to receiveEvent func
TEST_F(EventManagerTest, Registration)
{
  auto event_manager = std::make_unique<Event::Manager>();
  MockEventHandler event_handler;
  MockEventHandler event_handler_not_called;
  Event::Object test_event(Event::Type::NOOP);

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
  test_event.wait();

  // Unregister all event handlers before exiting
  event_manager->unregisterHandler(&event_handler);
}
