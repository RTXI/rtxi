/*
    lib/timer.c
    legacy timer crap

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



/* dt282x timer */

static int dt282x_timer(double freq,unsigned int *trigvar,double *actual_freq)
{
	int divider,prescaler;
	double basefreq=4e6;
	
	divider=floor(4e6/(freq));
	prescaler=0;
	while(divider>255){
		prescaler++;
		divider>>=1;
		basefreq/=2;
	}
	if(prescaler==1){
		prescaler++;
		divider>>=1;
		basefreq/=2;
	}
	if(prescaler>=16)return -1;
	*trigvar=(prescaler<<8)|(255-divider);
	*actual_freq=basefreq/divider;
	
	return 0;
}


/* dt2814 timer */
static int dt2814_timer(double freq,unsigned int *trigvar,double *actual_freq)
{
	double f;
	int  i;
	
	f=1e5;
	for(i=0;i<8;i++){
		if(f-freq<freq-f/10){
			*trigvar=i;
			*actual_freq=f;
			return 0;
		}
		f/=10;
	}
	*trigvar=i;
	*actual_freq=f;
	
	return 0;
}

/* atmio/pcimio timer */
static int atmio_timer(double freq,unsigned int *trigvar,double *actual_freq)
{
	unsigned int divider;
	
	divider=floor(20e6/(freq));
	*actual_freq=20e6/divider;
	*trigvar=divider-1;
	
	return 0;
}

/* acl8112 timer */
static int acl8112_timer(double freq,unsigned int *trigvar,double *actual_freq)
{
	int divider,prescaler;
	double basefreq=2e6;

	/* XXX my notes say that the prescaler and divider cannot
	   be 1.  This needs to be checked.  --ds */
	
	/* Force at least one division to get something in CTR2. */
	prescaler=1;
	divider = basefreq/(freq);

	while(divider>32767){
		prescaler*=2;
		divider>>=1;
	}

	*trigvar = (prescaler<<16) | divider;
	*actual_freq=basefreq/(divider*prescaler);
	
	return 0;
}

/* nanosec timer */
static int nanosec_timer(double freq,unsigned int *trigvar,double *actual_freq)
{
	*trigvar=(1e9/freq);
	*actual_freq=1e9/(*trigvar);

	return 0;
}

typedef int (*timerfunc)(double freq,unsigned int *trigvar,double *actual_freq);

static timerfunc timer_functions[]={
	NULL,
	dt282x_timer,
	dt2814_timer,
	atmio_timer,
	acl8112_timer,
	nanosec_timer,
};
#define N_TIMERTYPES 6

EXPORT_ALIAS_DEFAULT(_comedi_get_timer,comedi_get_timer,0.7.18);
int _comedi_get_timer(comedi_t *it,unsigned int subdev,double freq,
	unsigned int *trigvar,double *actual_freq)
{
	int timer_type;
	
	if(!valid_subd(it,subdev) || !trigvar || !actual_freq)
		return -1;

	timer_type=it->subdevices[subdev].timer_type;
	
	if(timer_type==0 || timer_type>=N_TIMERTYPES)
		return -1;
	
	return (timer_functions[timer_type])(freq,trigvar,actual_freq);
}

