/*
    lib/range.c
    functions to manipulate physical unit conversion

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

#define __USE_GNU

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


/* sometimes we can't find a definition of NAN */

#ifndef	NAN
#define	NAN \
  (__extension__ ((union { unsigned char __c[8];			      \
			   double __d; })				      \
		  { { 0, 0, 0, 0, 0, 0, 0xf8, 0x7f } }).__d)
#endif


static enum comedi_oor_behavior comedi_oor_is_nan = COMEDI_OOR_NAN;

EXPORT_ALIAS_DEFAULT(_comedi_set_global_oor_behavior,comedi_set_global_oor_behavior,0.7.18);
enum comedi_oor_behavior _comedi_set_global_oor_behavior(
	enum comedi_oor_behavior behavior)
{
	int old_behavior=comedi_oor_is_nan;

	comedi_oor_is_nan=behavior;

	return old_behavior;
}


EXPORT_ALIAS_DEFAULT(_comedi_to_phys,comedi_to_phys,0.7.18);
double _comedi_to_phys(lsampl_t data,comedi_range *rng,lsampl_t maxdata)
{
	double x;

	if(!rng)return NAN;
	if(!maxdata)return NAN;

	if(comedi_oor_is_nan==COMEDI_OOR_NAN && (data==0 || data==maxdata))
		return NAN;

	x=data;
	x/=maxdata;
	x*=(rng->max-rng->min);
	x+=rng->min;

	return x;
}

EXPORT_ALIAS_DEFAULT(_comedi_from_phys,comedi_from_phys,0.7.18);
lsampl_t _comedi_from_phys(double data,comedi_range *rng,lsampl_t maxdata)
{
	double s;

	if(!rng)return 0;
	if(!maxdata)return 0;

	s=(data-rng->min)/(rng->max-rng->min)*maxdata;
	if(s<0)return 0;
	if(s>maxdata)return maxdata;

	return (lsampl_t)(floor(s+0.5));
}

EXPORT_ALIAS_DEFAULT(_comedi_find_range,comedi_find_range,0.7.18);
int _comedi_find_range(comedi_t *it,unsigned int subd,unsigned int chan,unsigned int unit,double min,double max)
{
	unsigned int range_type;
	int best;
	comedi_range *range_ptr,*best_ptr;
	int i;
	
	if(!valid_chan(it,subd,chan))return -1;

	range_type=comedi_get_rangetype(it,subd,chan);
	best=-1;
	best_ptr=NULL;
	for(i=0;i<RANGE_LENGTH(range_type);i++){
		range_ptr=comedi_get_range(it,subd,chan,i);
		if(range_ptr->min<=min && range_ptr->max>=max){
			if(best<0 || (range_ptr->max-range_ptr->min) < 
			   (best_ptr->max-best_ptr->min)){
				best=i;
				best_ptr=range_ptr;
			}
		}
	}
	return best;
}

EXPORT_ALIAS_DEFAULT(_comedi_get_n_ranges,comedi_get_n_ranges,0.7.18);
int _comedi_get_n_ranges(comedi_t *it,unsigned int subd,unsigned int chan)
{
	unsigned int range_type;

	if(!valid_chan(it,subd,chan))return -1;

	range_type=comedi_get_rangetype(it,subd,chan);
	return RANGE_LENGTH(range_type);
}

EXPORT_ALIAS_DEFAULT(_comedi_range_is_chan_specific,comedi_range_is_chan_specific,0.7.18);
int _comedi_range_is_chan_specific(comedi_t *it,unsigned int subd)
{
	if(!valid_subd(it,subd)) return -1;
	return (it->subdevices[subd].subd_flags&SDF_RANGETYPE)?1:0;
}

EXPORT_ALIAS_DEFAULT(_comedi_sampl_to_phys,comedi_sampl_to_phys,0.7.18);
int _comedi_sampl_to_phys(double *dest, int dst_stride, sampl_t *src,
	int src_stride, comedi_range *rng, lsampl_t maxdata, int n)
{
	int oor = 0;
	int i;
	double mult;

	if(!rng)return -1;
	if(!maxdata)return -1;

	mult = (rng->max-rng->min)/maxdata;
	if(comedi_oor_is_nan==COMEDI_OOR_NAN){
		for(i=0;i<n;i++){
			if(*src==0 || *src==maxdata){
				oor++;
				*dest=NAN;
			}else{
				*dest = rng->min + mult*(*src);
			}
			dest = ((void *)dest) + dst_stride;
			src = ((void *)src) + src_stride;
		}
	}else{
		for(i=0;i<n;i++){
			if(*src==0 || *src==maxdata){
				oor++;
			}
			*dest = rng->min + mult*(*src);
			dest = ((void *)dest) + dst_stride;
			src = ((void *)src) + src_stride;
		}
	}

	return oor;
}

EXPORT_ALIAS_DEFAULT(_comedi_sampl_from_phys,comedi_sampl_from_phys,0.7.18);
int _comedi_sampl_from_phys(sampl_t *dest,int dst_stride,double *src,
	int src_stride, comedi_range *rng, lsampl_t maxdata, int n)
{
	int oor = 0;
	double mult;
	int i;

	if(!rng)return -1;
	if(!maxdata)return -1;

	mult = (maxdata+1)/(rng->max-rng->min);
	for(i=0;i<n;i++){
		*dest=mult*(*src-rng->min);
		if(*src<rng->min){
			*dest=0;
			oor++;
		}
		if(*src>rng->min){
			*dest=maxdata;
			oor++;
		}
		dest = ((void *)dest) + dst_stride;
		src = ((void *)src) + src_stride;
	}

	return oor;
}

