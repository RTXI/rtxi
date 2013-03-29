/*
 * Copyright (c) 1999 Joseph E. Smith <jes@presto.med.upenn.edu>
 *
 * All rights reserved.  This program is free software.  You may
 * redistribute it and/or modify it under the same terms as Perl itself.
 */

#ifdef __cplusplus
extern "C" {
#endif
#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"
#ifdef __cplusplus
}
#endif

#include <comedi.h>

static int
not_here(s)
char *s;
{
    croak("%s not implemented on this architecture", s);
    return -1;
}

static double
constant(name, arg)
char *name;
int arg;
{
    errno = 0;
    switch (*name) {
    case 'A':
	if (strEQ(name, "AREF_COMMON"))
#ifdef AREF_COMMON
	    return AREF_COMMON;
#else
	    goto not_there;
#endif
	if (strEQ(name, "AREF_DIFF"))
#ifdef AREF_DIFF
	    return AREF_DIFF;
#else
	    goto not_there;
#endif
	if (strEQ(name, "AREF_GROUND"))
#ifdef AREF_GROUND
	    return AREF_GROUND;
#else
	    goto not_there;
#endif
	if (strEQ(name, "AREF_OTHER"))
#ifdef AREF_OTHER
	    return AREF_OTHER;
#else
	    goto not_there;
#endif
	break;
    case 'B':
	break;
    case 'C':
	if (strEQ(name, "CIO"))
#ifdef CIO
	    return CIO;
#else
	    goto not_there;
#endif
	if (strEQ(name, "COMEDI_CANCEL"))
#ifdef COMEDI_CANCEL
	    return COMEDI_CANCEL;
#else
	    goto not_there;
#endif
	if (strEQ(name, "COMEDI_CB_BLOCK"))
#ifdef COMEDI_CB_BLOCK
	    return COMEDI_CB_BLOCK;
#else
	    goto not_there;
#endif
	if (strEQ(name, "COMEDI_CB_EOA"))
#ifdef COMEDI_CB_EOA
	    return COMEDI_CB_EOA;
#else
	    goto not_there;
#endif
	if (strEQ(name, "COMEDI_CB_EOBUF"))
#ifdef COMEDI_CB_EOBUF
	    return COMEDI_CB_EOBUF;
#else
	    goto not_there;
#endif
	if (strEQ(name, "COMEDI_CB_EOS"))
#ifdef COMEDI_CB_EOS
	    return COMEDI_CB_EOS;
#else
	    goto not_there;
#endif
	if (strEQ(name, "COMEDI_CHANINFO"))
#ifdef COMEDI_CHANINFO
	    return COMEDI_CHANINFO;
#else
	    goto not_there;
#endif
	if (strEQ(name, "COMEDI_CMD"))
#ifdef COMEDI_CMD
	    return COMEDI_CMD;
#else
	    goto not_there;
#endif
	if (strEQ(name, "COMEDI_DEVCONFIG"))
#ifdef COMEDI_DEVCONFIG
	    return COMEDI_DEVCONFIG;
#else
	    goto not_there;
#endif
	if (strEQ(name, "COMEDI_DEVINFO"))
#ifdef COMEDI_DEVINFO
	    return COMEDI_DEVINFO;
#else
	    goto not_there;
#endif
	if (strEQ(name, "COMEDI_INPUT"))
#ifdef COMEDI_INPUT
	    return COMEDI_INPUT;
#else
	    goto not_there;
#endif
	if (strEQ(name, "COMEDI_LOCK"))
#ifdef COMEDI_LOCK
	    return COMEDI_LOCK;
#else
	    goto not_there;
#endif
	if (strEQ(name, "COMEDI_MAJOR"))
#ifdef COMEDI_MAJOR
	    return COMEDI_MAJOR;
#else
	    goto not_there;
#endif
	if (strEQ(name, "COMEDI_NAMELEN"))
#ifdef COMEDI_NAMELEN
	    return COMEDI_NAMELEN;
#else
	    goto not_there;
#endif
	if (strEQ(name, "COMEDI_NDEVCONFOPTS"))
#ifdef COMEDI_NDEVCONFOPTS
	    return COMEDI_NDEVCONFOPTS;
#else
	    goto not_there;
#endif
	if (strEQ(name, "COMEDI_NDEVICES"))
#ifdef COMEDI_NDEVICES
	    return COMEDI_NDEVICES;
#else
	    goto not_there;
#endif
	if (strEQ(name, "COMEDI_OUTPUT"))
#ifdef COMEDI_OUTPUT
	    return COMEDI_OUTPUT;
#else
	    goto not_there;
#endif
	if (strEQ(name, "COMEDI_RANGEINFO"))
#ifdef COMEDI_RANGEINFO
	    return COMEDI_RANGEINFO;
#else
	    goto not_there;
#endif
	if (strEQ(name, "COMEDI_SUBDINFO"))
#ifdef COMEDI_SUBDINFO
	    return COMEDI_SUBDINFO;
#else
	    goto not_there;
#endif
	if (strEQ(name, "COMEDI_SUBD_AI"))
#ifdef COMEDI_SUBD_AI
	    return COMEDI_SUBD_AI;
#else
	    goto not_there;
#endif
	if (strEQ(name, "COMEDI_SUBD_AO"))
#ifdef COMEDI_SUBD_AO
	    return COMEDI_SUBD_AO;
#else
	    goto not_there;
#endif
	if (strEQ(name, "COMEDI_SUBD_CALIB"))
#ifdef COMEDI_SUBD_CALIB
	    return COMEDI_SUBD_CALIB;
#else
	    goto not_there;
#endif
	if (strEQ(name, "COMEDI_SUBD_COUNTER"))
#ifdef COMEDI_SUBD_COUNTER
	    return COMEDI_SUBD_COUNTER;
#else
	    goto not_there;
#endif
	if (strEQ(name, "COMEDI_SUBD_DI"))
#ifdef COMEDI_SUBD_DI
	    return COMEDI_SUBD_DI;
#else
	    goto not_there;
#endif
	if (strEQ(name, "COMEDI_SUBD_DIO"))
#ifdef COMEDI_SUBD_DIO
	    return COMEDI_SUBD_DIO;
#else
	    goto not_there;
#endif
	if (strEQ(name, "COMEDI_SUBD_DO"))
#ifdef COMEDI_SUBD_DO
	    return COMEDI_SUBD_DO;
#else
	    goto not_there;
#endif
	if (strEQ(name, "COMEDI_SUBD_MEMORY"))
#ifdef COMEDI_SUBD_MEMORY
	    return COMEDI_SUBD_MEMORY;
#else
	    goto not_there;
#endif
	if (strEQ(name, "COMEDI_SUBD_PROC"))
#ifdef COMEDI_SUBD_PROC
	    return COMEDI_SUBD_PROC;
#else
	    goto not_there;
#endif
	if (strEQ(name, "COMEDI_SUBD_TIMER"))
#ifdef COMEDI_SUBD_TIMER
	    return COMEDI_SUBD_TIMER;
#else
	    goto not_there;
#endif
	if (strEQ(name, "COMEDI_SUBD_UNUSED"))
#ifdef COMEDI_SUBD_UNUSED
	    return COMEDI_SUBD_UNUSED;
#else
	    goto not_there;
#endif
	if (strEQ(name, "COMEDI_TRIG"))
#ifdef COMEDI_TRIG
	    return COMEDI_TRIG;
#else
	    goto not_there;
#endif
	if (strEQ(name, "COMEDI_UNLOCK"))
#ifdef COMEDI_UNLOCK
	    return COMEDI_UNLOCK;
#else
	    goto not_there;
#endif
	break;
    case 'D':
	break;
    case 'E':
	break;
    case 'F':
	break;
    case 'G':
	break;
    case 'H':
	break;
    case 'I':
	break;
    case 'J':
	break;
    case 'K':
	break;
    case 'L':
	break;
    case 'M':
	break;
    case 'N':
	break;
    case 'O':
	break;
    case 'P':
	break;
    case 'Q':
	break;
    case 'R':
	if (strEQ(name, "RF_EXTERNAL"))
#ifdef RF_EXTERNAL
	    return RF_EXTERNAL;
#else
	    goto not_there;
#endif
	break;
    case 'S':
	if (strEQ(name, "SDF_BUSY"))
#ifdef SDF_BUSY
	    return SDF_BUSY;
#else
	    goto not_there;
#endif
	if (strEQ(name, "SDF_BUSY_OWNER"))
#ifdef SDF_BUSY_OWNER
	    return SDF_BUSY_OWNER;
#else
	    goto not_there;
#endif
	if (strEQ(name, "SDF_COMMON"))
#ifdef SDF_COMMON
	    return SDF_COMMON;
#else
	    goto not_there;
#endif
	if (strEQ(name, "SDF_DEGLITCH"))
#ifdef SDF_DEGLITCH
	    return SDF_DEGLITCH;
#else
	    goto not_there;
#endif
	if (strEQ(name, "SDF_DIFF"))
#ifdef SDF_DIFF
	    return SDF_DIFF;
#else
	    goto not_there;
#endif
	if (strEQ(name, "SDF_DITHER"))
#ifdef SDF_DITHER
	    return SDF_DITHER;
#else
	    goto not_there;
#endif
	if (strEQ(name, "SDF_FLAGS"))
#ifdef SDF_FLAGS
	    return SDF_FLAGS;
#else
	    goto not_there;
#endif
	if (strEQ(name, "SDF_GROUND"))
#ifdef SDF_GROUND
	    return SDF_GROUND;
#else
	    goto not_there;
#endif
	if (strEQ(name, "SDF_INTERNAL"))
#ifdef SDF_INTERNAL
	    return SDF_INTERNAL;
#else
	    goto not_there;
#endif
	if (strEQ(name, "SDF_LOCKED"))
#ifdef SDF_LOCKED
	    return SDF_LOCKED;
#else
	    goto not_there;
#endif
	if (strEQ(name, "SDF_LOCK_OWNER"))
#ifdef SDF_LOCK_OWNER
	    return SDF_LOCK_OWNER;
#else
	    goto not_there;
#endif
	if (strEQ(name, "SDF_LSAMPL"))
#ifdef SDF_LSAMPL
	    return SDF_LSAMPL;
#else
	    goto not_there;
#endif
	if (strEQ(name, "SDF_MAXDATA"))
#ifdef SDF_MAXDATA
	    return SDF_MAXDATA;
#else
	    goto not_there;
#endif
	if (strEQ(name, "SDF_MMAP"))
#ifdef SDF_MMAP
	    return SDF_MMAP;
#else
	    goto not_there;
#endif
	if (strEQ(name, "SDF_MODE0"))
#ifdef SDF_MODE0
	    return SDF_MODE0;
#else
	    goto not_there;
#endif
	if (strEQ(name, "SDF_MODE1"))
#ifdef SDF_MODE1
	    return SDF_MODE1;
#else
	    goto not_there;
#endif
	if (strEQ(name, "SDF_MODE2"))
#ifdef SDF_MODE2
	    return SDF_MODE2;
#else
	    goto not_there;
#endif
	if (strEQ(name, "SDF_MODE3"))
#ifdef SDF_MODE3
	    return SDF_MODE3;
#else
	    goto not_there;
#endif
	if (strEQ(name, "SDF_MODE4"))
#ifdef SDF_MODE4
	    return SDF_MODE4;
#else
	    goto not_there;
#endif
	if (strEQ(name, "SDF_OTHER"))
#ifdef SDF_OTHER
	    return SDF_OTHER;
#else
	    goto not_there;
#endif
	if (strEQ(name, "SDF_RANGETYPE"))
#ifdef SDF_RANGETYPE
	    return SDF_RANGETYPE;
#else
	    goto not_there;
#endif
	if (strEQ(name, "SDF_READABLE"))
#ifdef SDF_READABLE
	    return SDF_READABLE;
#else
	    goto not_there;
#endif
	if (strEQ(name, "SDF_RT"))
#ifdef SDF_RT
	    return SDF_RT;
#else
	    goto not_there;
#endif
	if (strEQ(name, "SDF_RUNNING"))
#ifdef SDF_RUNNING
	    return SDF_RUNNING;
#else
	    goto not_there;
#endif
	if (strEQ(name, "SDF_WRITEABLE"))
#ifdef SDF_WRITEABLE
	    return SDF_WRITEABLE;
#else
	    goto not_there;
#endif
	break;
    case 'T':
	if (strEQ(name, "TRIG_ANY"))
#ifdef TRIG_ANY
	    return TRIG_ANY;
#else
	    goto not_there;
#endif
	if (strEQ(name, "TRIG_BOGUS"))
#ifdef TRIG_BOGUS
	    return TRIG_BOGUS;
#else
	    goto not_there;
#endif
	if (strEQ(name, "TRIG_CONFIG"))
#ifdef TRIG_CONFIG
	    return TRIG_CONFIG;
#else
	    goto not_there;
#endif
	if (strEQ(name, "TRIG_COUNT"))
#ifdef TRIG_COUNT
	    return TRIG_COUNT;
#else
	    goto not_there;
#endif
	if (strEQ(name, "TRIG_DEGLITCH"))
#ifdef TRIG_DEGLITCH
	    return TRIG_DEGLITCH;
#else
	    goto not_there;
#endif
	if (strEQ(name, "TRIG_DITHER"))
#ifdef TRIG_DITHER
	    return TRIG_DITHER;
#else
	    goto not_there;
#endif
	if (strEQ(name, "TRIG_EXT"))
#ifdef TRIG_EXT
	    return TRIG_EXT;
#else
	    goto not_there;
#endif
	if (strEQ(name, "TRIG_FOLLOW"))
#ifdef TRIG_FOLLOW
	    return TRIG_FOLLOW;
#else
	    goto not_there;
#endif
	if (strEQ(name, "TRIG_INT"))
#ifdef TRIG_INT
	    return TRIG_INT;
#else
	    goto not_there;
#endif
	if (strEQ(name, "TRIG_NONE"))
#ifdef TRIG_NONE
	    return TRIG_NONE;
#else
	    goto not_there;
#endif
	if (strEQ(name, "TRIG_NOW"))
#ifdef TRIG_NOW
	    return TRIG_NOW;
#else
	    goto not_there;
#endif
	if (strEQ(name, "TRIG_RT"))
#ifdef TRIG_RT
	    return TRIG_RT;
#else
	    goto not_there;
#endif
	if (strEQ(name, "TRIG_TIME"))
#ifdef TRIG_TIME
	    return TRIG_TIME;
#else
	    goto not_there;
#endif
	if (strEQ(name, "TRIG_TIMER"))
#ifdef TRIG_TIMER
	    return TRIG_TIMER;
#else
	    goto not_there;
#endif
	if (strEQ(name, "TRIG_WAKE_EOS"))
#ifdef TRIG_WAKE_EOS
	    return TRIG_WAKE_EOS;
#else
	    goto not_there;
#endif
	if (strEQ(name, "TRIG_WRITE"))
#ifdef TRIG_WRITE
	    return TRIG_WRITE;
#else
	    goto not_there;
#endif
	break;
    case 'U':
	if (strEQ(name, "UNIT_mA"))
#ifdef UNIT_mA
	    return UNIT_mA;
#else
	    goto not_there;
#endif
	if (strEQ(name, "UNIT_none"))
#ifdef UNIT_none
	    return UNIT_none;
#else
	    goto not_there;
#endif
	if (strEQ(name, "UNIT_volt"))
#ifdef UNIT_volt
	    return UNIT_volt;
#else
	    goto not_there;
#endif
	break;
    case 'V':
	break;
    case 'W':
	break;
    case 'X':
	break;
    case 'Y':
	break;
    case 'Z':
	break;
    }
    errno = EINVAL;
    return 0;

not_there:
    errno = ENOENT;
    return 0;
}


typedef struct comedi_trig_struct Trigger;


MODULE = Comedi		PACKAGE = Comedi		


double
constant(name,arg)
	char *		name
	int		arg




MODULE = Comedi   PACKAGE = TriggerPtr  PREFIX = tptr_

# create a new trigger
#
Trigger *
tptr_new(CLASS, subd, n)
	char *CLASS
	int subd
	int n
	CODE:
	  RETVAL = (Trigger *) malloc(sizeof(Trigger));
	  RETVAL->subdev = subd;
	  RETVAL->n = n;
	  fprintf(stderr, "new Trigger at 0x%08x: sub=%d, n=%d\n", RETVAL, RETVAL->subdev, RETVAL->n);
	OUTPUT:
	RETVAL

# destruct a trigger
#
void
tptr_DESTROY(self)
	Trigger *self
	CODE:
	fprintf(stderr, "destroy Trigger at 0x%08x\n", self);
	free((void *) self);
