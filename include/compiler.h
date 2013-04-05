/*
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

#ifndef COMPILER_H
#define COMPILER_H

#define likely(x)   __builtin_expect((x),1)
#define unlikely(x) __builtin_expect((x),0)

#define prefetch_read(x)  __builtin_prefetch((x),0,1)
#define prefetch_write(x) __builtin_prefetch((x),1,1)

#endif // COMPILER_H
