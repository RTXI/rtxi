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

#ifndef RT_OS_TESTS_H
#define RT_OS_TESTS_H

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "rt.hpp"

#include <vector>
#include <memory>

class RTOSTests : public ::testing::Test
{
protected:
  RTOSTests()=default;
  ~RTOSTests()=default;

};

void temp_function(size_t bytes, int* retval);
#endif