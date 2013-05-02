/*
    lib/timed.c
    description

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

#include "libinternal.h"

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


#define BUFSZ 100

EXPORT_ALIAS_DEFAULT(_comedi_timed_1chan,comedi_timed_1chan,0.7.18);
int _comedi_timed_1chan(comedi_t *dev,unsigned int subd,unsigned int chan,unsigned int range,
	unsigned int aref,double freq,unsigned int n_samples,double *data)
{
	comedi_trig t;
	double act_freq;
	sampl_t *buffer;
	comedi_range *the_range;
	unsigned int maxdata;
	int i,n,m;
	
	if(!valid_chan(dev,subd,chan))return -1;
	if(!data)return -1;
	
	memset(&t,0,sizeof(t));
	
	/* check range */

	the_range=comedi_get_range(dev,subd,chan,range);
	maxdata=comedi_get_maxdata(dev,subd,chan);

	chan=CR_PACK(chan,range,aref);

	t.subdev=subd;
	t.mode=2;
	t.n_chan=1;
	t.chanlist=&chan;
	t.n=n_samples;
	comedi_get_timer(dev,subd,freq,&t.trigvar,&act_freq);
	t.trigvar1=1;
	
	buffer=malloc(sizeof(sampl_t)*BUFSZ);
	if(!buffer)return -1;

	comedi_trigger(dev,&t);
	n=0;
	while(n<n_samples){
		m=n_samples-n;
		if(m>BUFSZ)m=BUFSZ;
		if((m=read(dev->fd,buffer,m*sizeof(sampl_t)))<0){
			/* ack! */
			return -1;
		}
		m/=sizeof(sampl_t);
		for(i=0;i<m;i++){
			data[n+i]=comedi_to_phys(buffer[i],the_range,maxdata);
		}
		n+=m;
	}

	free(buffer);

	return 0;
}

