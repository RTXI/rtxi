/*
 * Copyright (C) 2004 Boston University
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License as
 *  published by the Free Software Foundation; either version 2 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef DEBUG_H
#define DEBUG_H

#include <execinfo.h>
#include <stdio.h>

//! Prints a backtrace to standard error.
static inline void PRINT_BACKTRACE(void) {
    int buffer_size;
    void *buffer[256];

    buffer_size = backtrace(buffer,sizeof(buffer));
    fprintf(stderr,"Backtrace:\n");
    backtrace_symbols_fd(buffer,buffer_size,2);
}

#define ERROR_MSG(fmt,args...) do { fprintf(stderr,"%s:%d:",__FILE__,__LINE__); fprintf(stderr,fmt,## args); } while(0)

#ifdef DEBUG

#define DEBUG_MSG(fmt,args...) do { fprintf(stderr,"%s:%d:",__FILE__,__LINE__); fprintf(stderr,fmt,## args); } while(0)

#else /* !DEBUG */

//! Prints debug messages to standard error.
/*!
 * When compiled without the DEBUG flag messages compile out.
 */
#define DEBUG_MSG(fmt,args...) do { ; } while(0)

#endif /* DEBUG */

#endif /* DEBUG_H */
