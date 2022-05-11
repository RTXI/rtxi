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

#include <sys/resource.h>
#include <sys/time.h>
#include <sys/errno.h>

#include "rt_os_tests.hpp"

std::vector<char>* RTOSTests::test_function(size_t bytes)
{
    auto *retval = new std::vector<char>();
    retval->reserve(bytes);
    return retval;
}

TEST_F(RTOSTests, initiate_and_shutdown)
{
    struct rlimit rlim;
    int res = getrlimit(RLIMIT_MEMLOCK, &rlim);
    std::vector<char> *data = test_function(rlim.rlim_cur-1);
    ASSERT_EQ(RT::OS::initiate(), 0);
    delete data;
    data = test_function(rlim.rlim_cur+1);
    ASSERT_EQ(RT::OS::initiate, -1);
    delete data;
}

TEST_F(RTOSTests, createTask)
{
}

TEST_F(RTOSTests, setPeriod)
{
}

TEST_F(RTOSTests, sleepTimestep)
{
}

TEST_F(RTOSTests, isRealtime)
{
}

TEST_F(RTOSTests, getTime)
{
}

TEST_F(RTOSTests, getCpuUsage)
{
}

