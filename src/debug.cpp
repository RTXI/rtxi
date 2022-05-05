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

#include <iostream>
#include "debug.hpp"

//
// void PRINT_BACKTRACE(void)
//{
//    std::cerr << boost::stacktrace::stacktrace();
//}

void ERROR_MSG(const std::string errmsg, ...)
{
  va_list args;
  va_start(args, errmsg);
  char buf[256] = "";
  vsprintf(buf, errmsg.c_str(), args);
  std::cerr << std::string(buf, 256) << "\n";
  va_end(args);
}
