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
#include <functional>
#include <chrono>

#include "rt_os_tests.hpp"


void temp_function(int& retval)
{
  retval = RT::OS::initiate();
  RT::OS::shutdown();
}

TEST_F(RTOSTests, InitiateAndShutdown) {
  int result = 0;
  std::thread temp_thread(
    [](int& retval){
      retval = RT::OS::initiate();
      RT::OS::shutdown();
    }, 
    std::ref(result)
  );
  temp_thread.join();
  // It is not possible to lock memory without admin privilages. 
  EXPECT_TRUE(result == 0 || result == -1);
}

TEST_F(RTOSTests, CreateAndDeleteTask) {
  using namespace std::chrono_literals;
  int result = 0;
  RT::OS::Task *test_task;
  result = createTask(
    test_task, 
    [](int& retval){
      std::this_thread::sleep_for(1s);
      retval = 1;
    }, 
    &result, 
    0);

}

TEST_F(RTOSTests, setPeriod) {}

TEST_F(RTOSTests, sleepTimestep) {}

TEST_F(RTOSTests, isRealtime) {}

TEST_F(RTOSTests, getTime) {}

TEST_F(RTOSTests, getCpuUsage) {}
