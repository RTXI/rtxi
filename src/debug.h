/*
	 The Real-Time eXperiment Interface (RTXI)
	 Copyright (C) 2011 Georgia Institute of Technology, University of Utah, Weill Cornell Medical College

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

#ifndef DEBUG_H
#define DEBUG_H

#include <boost/stacktrace.hpp>
#include <execinfo.h>
//#include <stdio.h>
#include <iostream>
#include <string>
#include <memory>

#if XENOMAI
#include <rtdk.h>
#endif

//! Prints a backtrace to standard error.
static inline void PRINT_BACKTRACE(void)
{
    std::cerr << boost::stacktrace::stacktrace();
}

template<typename... Args>
void ERROR_MSG(const std::string& errmsg, Args... args){
    auto size = errmsg.size();
    std::unique_ptr<char[]> buf = std::make_unique<char[]>(size);
    std::snprintf(buf.get(), size, errmsg.c_str(), args ...);
    std::cerr << std::string(buf.get(), buf.get() + size) << "\n";
    std::cerr << boost::stacktrace::stacktrace();
}

#endif /* DEBUG_H */
