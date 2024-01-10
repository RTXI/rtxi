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

#include "debug.hpp"
#include <array>

void PRINT_BACKTRACE()
{
  int buffer_size = 0;
  std::array<void*, 256> buffer{};

  buffer_size = backtrace(buffer.data(),buffer.size()*sizeof(void*));
  ERROR_MSG("Backtrace:\n");
  backtrace_symbols_fd(buffer.data(),buffer_size,2);
}

