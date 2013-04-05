/*
    lib/ioctl.c
    low-level functions

    COMEDILIB - Linux Control and Measurement Device Interface Library
    Copyright (C) 1997-2001 David A. Schleef <ds@schleef.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation, version 2.1
    of the License.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307
    USA.
*/

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>

#include "libinternal.h"


/* ioctl wrappers */

int _comedi_ioctl(int fd, int request, void *arg)
{
	int ret;

	ret = ioctl(fd, request, arg);
	if(ret < 0)
		libc_error();
	return ret;
}

int _comedi_ioctl_debug(int fd, int request, void *arg)
{
	int ret;

	fprintf(stderr,"ioctl(%d,0x%08x,%p) = ",fd, request, arg);
	ret = _comedi_ioctl(fd, request, arg);
	fprintf(stderr,"%d\n", ret);

	return ret;
}


