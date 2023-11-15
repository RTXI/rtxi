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

#ifndef FIFO_TESTS_H
#define FIFO_TESTS_H

#include <memory>
#include <string>

#include <gtest/gtest.h>

#include "fifo.hpp"

class FifoTest : public ::testing::Test
{
protected:
  size_t default_buffer_size = 100;
  std::string default_message = "message";
};

#endif
