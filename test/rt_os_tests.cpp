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
#include <functional>
#include <thread>

#include "rt_os_tests.hpp"

void temp_function(bool& retval)
{
  retval = true;
}

TEST_F(RTOSTests, InitiateAndShutdown)
{
  int result = 0;
  std::thread temp_thread([&result]()
    {
      result = RT::OS::initiate();
      RT::OS::shutdown();
    }
  );
  temp_thread.join();
  // It is not possible to lock memory without admin privilages.
  // Either it succeeds or we don't have permissions
  EXPECT_TRUE(result == 11 || result == -13 || result == 0);
}

TEST_F(RTOSTests, CreateAndDeleteTask)
{
  int result = 0;
  bool func_retval = false;
  auto test_task = std::make_unique<RT::OS::Task>();
  result = RT::OS::createTask<bool>(test_task, temp_function, func_retval);
  RT::OS::deleteTask(test_task);
  // It is not possible to lock memory without admin privilages.
  // Either it succeeds or we don't have permissions
  EXPECT_TRUE(result == 0 || result == -13);
}

TEST_F(RTOSTests, setPeriod) 
{
  auto test_task = std::make_unique<RT::OS::Task>();
  ASSERT_EQ(RT::OS::DEFAULT_PERIOD, test_task->period);
  int64_t period = RT::OS::DEFAULT_PERIOD * 2;
  RT::OS::setPeriod(test_task, period);
  ASSERT_EQ(period, test_task->period);
}

TEST_F(RTOSTests, getTime) 
{
  auto nsec = RT::OS::getTime();
  ASSERT_GT(RT::OS::getTime(), nsec);
}

TEST_F(RTOSTests, sleepTimestep) 
{
  auto test_task = std::make_unique<RT::OS::Task>();
  ASSERT_EQ(test_task->next_t, 0);
  int resval = RT::OS::initiate();
  auto stime = RT::OS::getTime();
  RT::OS::sleepTimestep(test_task);
  auto etime = RT::OS::getTime();
  auto duration = etime - stime;
  RT::OS::shutdown();
  //if (resval)
  ASSERT_GE(duration, test_task->period);
  ASSERT_NE(test_task->next_t, 0);
}

TEST_F(RTOSTests, isRealtime) 
{
  ASSERT_EQ(false, RT::OS::isRealtime());
}

// TODO: Create test for cpu usage modules
TEST_F(RTOSTests, getCpuUsage) {}
