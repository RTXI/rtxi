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

#include <chrono>
#include <thread>

#include <gmock/gmock.h>

#include "system_tests.hpp"

TEST_F(SystemTest, checkTelemitry)
{
  Event::Object event(Event::Type::NOOP);
  this->system->receiveEvent(&event);
  event.wait();
  ASSERT_EQ(RT::Telemitry::RT_NOOP, this->system->getTelemitry());
}

TEST_F(SystemTest, getPeriod)
{
  // Check with default period
  auto period = 1000000ll;
  ASSERT_EQ(period, system->getPeriod());
}

TEST_F(SystemTest, setPeriod)
{
  Event::Object ev(Event::Type::RT_PERIOD_EVENT);
  ev.setParam("period", RT::OS::DEFAULT_PERIOD/2);
  this->system->receiveEvent(&ev);
  ev.wait();
  EXPECT_EQ(RT::Telemitry::RT_PERIOD_UPDATE, this->system->getTelemitry());
  ASSERT_EQ(RT::OS::DEFAULT_PERIOD/2, system->getPeriod());
  ev.setParam("period", RT::OS::DEFAULT_PERIOD);
  this->system->receiveEvent(&ev);
  ev.wait();
  EXPECT_EQ(RT::Telemitry::RT_PERIOD_UPDATE, this->system->getTelemitry());
  ASSERT_EQ(RT::OS::DEFAULT_PERIOD, system->getPeriod());
}
