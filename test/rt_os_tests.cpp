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

#include <malloc.h>
#include <sys/time.h>
#include <sys/resource.h>

#include <thread>
#include <string>

#include "rt_os_tests.hpp"


void temp_function(size_t bytes, int* retval)
{
  *retval = RT::OS::initiate();
  RT::OS::shutdown();
}

TEST_F(RTOSTests, InitiateAndShutdown) {
  int result;
  std::thread temp_thread(&temp_function, 1, &result);
  temp_thread.join();
  EXPECT_EQ(result, 0);
}

TEST_F(RTOSTests, createTask_and_deleteTask) {

}

TEST_F(RTOSTests, setPeriod) {}

TEST_F(RTOSTests, sleepTimestep) {}

TEST_F(RTOSTests, isRealtime) {}

TEST_F(RTOSTests, getTime) {}

TEST_F(RTOSTests, getCpuUsage) {}
