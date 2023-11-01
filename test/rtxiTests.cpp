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

#include <iostream>

#include "rtxiTests.h"

#include <boost/stacktrace.hpp>
#include <signal.h>

#include "debug.hpp"

void TestEnvironment::SetUp()
{
  argc = 3;
  char* argv[3];
  argv[0] = const_cast<char*>("TestQApp");
  argv[1] = const_cast<char*>("-platform");
  argv[2] = const_cast<char*>("offscreen");
  app = new QApplication(argc, argv);
  if (app != nullptr) {
    std::cout << "QT Application object initialized." << std::endl;
  } else {
    std::cerr << "Unable to initialized QT application." << std::endl;
  }
}

namespace
{
void signal_handler(int signum)
{
  // NOLINTNEXTLINE
  ERROR_MSG("signal_handler : signal type {} received\n", signum);
  std::cerr << boost::stacktrace::stacktrace();
  exit(-1);  // NOLINT
}
}  // namespace

void TestEnvironment::TearDown()
{
  delete app;
}

int main(int argc, char* argv[])
{
  if (signal(SIGINT, signal_handler) == SIG_ERR) {
    ERROR_MSG("MAIN: Unable to set SIGINT signal handler");
    return -1;
  }
  if (signal(SIGABRT, signal_handler) == SIG_ERR) {
    ERROR_MSG("MAIN: Unable to set SIGABRT signal handler");
    return -1;
  }
  if (signal(SIGSEGV, signal_handler) == SIG_ERR) {
    ERROR_MSG("MAIN: Unable to set SIGSEGV signal handler");
    return -1;
  }
  ::testing::InitGoogleTest(&argc, argv);
  ::testing::AddGlobalTestEnvironment(new TestEnvironment());
  return RUN_ALL_TESTS();
}
