/*
    comedi_errno.h
    header file for comedi's symbolic error names

    COMEDI - Linux Control and Measurement Device Interface
    Copyright (C) 1997-2000 David A. Schleef <ds@schleef.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as
    ublished by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Cambridge, MA 02111, USA.

*/

#ifndef _COMEDI_ERRNO_H
#define _COMEDI_ERRNO_H

#ifdef __KERNEL__
#include <linux/errno.h>
#else
#include <errno.h>
#endif

// define symbolic error names, extensions peculiar to comedi
#define COMEDI_NOERROR		0x1000

#define EUNKNOWN		(COMEDI_NOERROR + 1)		// unknown error
#define EBAD_CT			(COMEDI_NOERROR + 2)		// bad comedi_t struct
#define EINVAL_SUBD		(COMEDI_NOERROR + 3)		// invalid subdevice
#define EINVAL_CHAN		(COMEDI_NOERROR + 4)		// invalid channel
#define EBUF_OVR		(COMEDI_NOERROR + 5)		// buffer overflow
#define EBUF_UNDR		(COMEDI_NOERROR + 6)		// buffer underflow
#define ECMDNOTSUPP		(COMEDI_NOERROR + 7)		// command not supported
#define ENOTSUPPORTED		(COMEDI_NOERROR + 8)		// not supported

#endif	// _COMEDI_ERRNO_H
