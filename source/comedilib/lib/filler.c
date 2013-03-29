/*
    lib/filler.c
    functions to retrieve kernel data

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

#include <assert.h>
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


/* these functions download information from the comedi module. */

static int do_test_for_cmd(comedi_t *dev,unsigned int subdevice);
static int do_test_for_insn(comedi_t *dev);
static int do_test_for_insnlist(comedi_t *dev);
static int do_test_for_insn_bits(comedi_t *dev,unsigned int subdevice);


int get_subdevices(comedi_t *it)
{
	int i,j;
	int ret;
	comedi_subdinfo *s;
	comedi_chaninfo ci;
	subdevice *r = NULL;

	s = calloc(it->n_subdevices, sizeof(comedi_subdinfo));
	if(s == NULL)
	{
		debug_ptr(s);
		libc_error();
		goto cleanup;
	}

	ret = comedi_ioctl(it->fd, COMEDI_SUBDINFO, s);
	if(ret < 0)
	{
		debug_int(ret);
		goto cleanup;
	}

	assert(it->subdevices == NULL);
	r = it->subdevices = calloc(it->n_subdevices, sizeof(subdevice));
	if(r == NULL)
	{
		debug_ptr(r);
		libc_error();
		goto cleanup;
	}

	it->has_insnlist_ioctl = do_test_for_insnlist(it);
	it->has_insn_ioctl = do_test_for_insn(it);
	for(i=0;i<it->n_subdevices;i++){
		r[i].type	= s[i].type;
		if(r[i].type==COMEDI_SUBD_UNUSED)continue;
		r[i].n_chan	= s[i].n_chan;
		r[i].subd_flags	= s[i].subd_flags;
		r[i].timer_type	= s[i].timer_type;
		r[i].len_chanlist = s[i].len_chanlist;
		r[i].maxdata	= s[i].maxdata;
		r[i].flags	= s[i].flags;
		r[i].range_type = s[i].range_type;

		if(r[i].subd_flags&SDF_FLAGS){
			r[i].flags_list = calloc(r[i].n_chan, sizeof(*r[i].flags_list));
			if(r[i].flags_list == NULL){
				debug_ptr(r[i].flags_list);
				libc_error();
				goto cleanup;
			}

		}
		if(r[i].subd_flags&SDF_MAXDATA){
			r[i].maxdata_list = calloc(r[i].n_chan, sizeof(*r[i].maxdata_list));
			if(r[i].maxdata_list == NULL){
				debug_ptr(r[i].maxdata_list);
				libc_error();
				goto cleanup;
			}
		}
		if(r[i].subd_flags&SDF_RANGETYPE){
			r[i].range_type_list = calloc(r[i].n_chan, sizeof(*r[i].range_type_list));
			if(r[i].range_type_list == NULL){
				debug_ptr(r[i].range_type_list);
				libc_error();
				goto cleanup;
			}
		}
		ci.subdev = i;
		ci.flaglist = r[i].flags_list;
		ci.rangelist = r[i].range_type_list;
		ci.maxdata_list = r[i].maxdata_list;
		ret = comedi_ioctl(it->fd, COMEDI_CHANINFO, &ci);
		if(ret < 0){
			debug_int(ret);
			goto cleanup;
		}

		if(r[i].subd_flags&SDF_RANGETYPE){
			r[i].rangeinfo_list=calloc(r[i].n_chan, sizeof(*r[i].rangeinfo_list));
			if(r[i].rangeinfo_list == NULL){
				debug_ptr(r[i].rangeinfo_list);
				libc_error();
				goto cleanup;
			}
			for(j=0;j<r[i].n_chan;j++){
				r[i].rangeinfo_list[j]=get_rangeinfo(it->fd,r[i].range_type_list[j]);
				if(r[i].rangeinfo_list[j] == NULL)
					goto cleanup;
			}
		}else{
			r[i].rangeinfo=get_rangeinfo(it->fd,r[i].range_type);
			if(r[i].rangeinfo == NULL)
				goto cleanup;
		}

		r[i].has_cmd = do_test_for_cmd(it,i);
		switch(s[i].insn_bits_support)
		{
		case COMEDI_UNKNOWN_SUPPORT:
			if(it->has_insnlist_ioctl){
				r[i].has_insn_bits = do_test_for_insn_bits(it,i);
			}else{
				r[i].has_insn_bits = 0;
			}
			break;
		case COMEDI_SUPPORTED:
			r[i].has_insn_bits = 1;
			break;
		case COMEDI_UNSUPPORTED:
			r[i].has_insn_bits = 0;
			break;
		default:
			assert(0);
		}
	}

	free(s);

	return 0;

cleanup:

	if(s)
		free(s);

	if(r){
		for(i=0;i<it->n_subdevices;i++){
			if(r[i].flags_list)
				free(r[i].flags_list);
			if(r[i].maxdata_list)
				free(r[i].maxdata_list);
			if(r[i].range_type_list)
				free(r[i].range_type_list);
			if(r[i].rangeinfo_list){
				for(j=0;j<r[i].n_chan;j++){
					if(r[i].rangeinfo_list[j])
						free(r[i].rangeinfo_list[j]);
				}
			}else{
				if(r[i].rangeinfo)
					free(r[i].rangeinfo);
			}
		}
		free(r);
		it->subdevices = NULL;
	}

	return -1;
}

comedi_range *get_rangeinfo(int fd,unsigned int range_type)
{
	comedi_krange *kr;
	comedi_range *r;
	comedi_rangeinfo ri;
	int ret;
	int i;

	kr = calloc(RANGE_LENGTH(range_type), sizeof(comedi_krange));
	if(kr == NULL)
	{
		debug_ptr(kr);
		libc_error();
		return NULL;
	}
	r = calloc(RANGE_LENGTH(range_type), sizeof(comedi_range));
	if(r == NULL)
	{
		debug_ptr(r);
		libc_error();
		free(kr);
		return NULL;
	}

	memset(&ri, 0, sizeof(ri));
	ri.range_type = range_type;
	ri.range_ptr = kr;
	ret = comedi_ioctl(fd, COMEDI_RANGEINFO, &ri);
	if(ret<0){
		fprintf(stderr,"ioctl(%d,COMEDI_RANGEINFO,0x%08x,%p)\n",fd,range_type,kr);
		free(r);
		free(kr);
		return NULL;
	}

	for(i=0;i<RANGE_LENGTH(range_type);i++){
		r[i].min=kr[i].min*1e-6;
		r[i].max=kr[i].max*1e-6;
		r[i].unit=kr[i].flags;
	}
	free(kr);

	return r;
}


/* some command testing */

static int do_test_for_cmd(comedi_t *dev,unsigned int subdevice)
{
	/* SDF_CMD was added in 0.7.57 */
	if(dev->devinfo.version_code >= COMEDI_VERSION_CODE(0,7,57)){
		if(dev->subdevices[subdevice].subd_flags & SDF_CMD)
			return 1;
		return 0;
	}else{
		comedi_cmd it;
		int ret;

		memset(&it,0,sizeof(it));

		it.subdev = subdevice;
		it.start_src = TRIG_ANY;
		it.scan_begin_src = TRIG_ANY;
		it.convert_src = TRIG_ANY;
		it.scan_end_src = TRIG_ANY;
		it.stop_src = TRIG_ANY;

		ret = comedi_ioctl(dev->fd, COMEDI_CMDTEST, &it);

		if(ret<0 && errno==EIO){
			return 0;
		}
		if(ret<0){
			fprintf(stderr,"BUG in do_test_for_cmd()\n");
			return 0;
		}
		return 1;
	}
}

static int do_test_for_insnlist(comedi_t *dev)
{
	comedi_insn insn;
	comedi_insnlist il;
	lsampl_t data[2];
	int ret;

	il.n_insns = 1;
	il.insns = &insn;

	memset(&insn,0,sizeof(insn));
	insn.insn = INSN_GTOD;
	insn.n = 2;
	insn.data = data;
	memset(insn.data, 0, insn.n * sizeof(insn.data[0]));

	ret = comedi_ioctl(dev->fd, COMEDI_INSNLIST, &il);

	if(ret<0){
		if(errno!=EIO){
			fprintf(stderr,"BUG in do_test_for_insnlist()\n");
		}
		return 0;
	}
	return 1;
}

/* the COMEID_INSN ioctl was introduced in comedi-0.7.60 */
static int do_test_for_insn(comedi_t *dev)
{
	comedi_insn insn;
	lsampl_t data[2];
	int ret;

	memset(&insn,0,sizeof(insn));
	insn.insn = INSN_GTOD;
	insn.n = 2;
	insn.data = data;
	memset(insn.data, 0, insn.n * sizeof(insn.data[0]));

	ret = comedi_ioctl(dev->fd, COMEDI_INSN, &insn);

	if(ret<0){
		if(errno!=EIO){
			fprintf(stderr,"BUG in do_test_for_insn()\n");
		}
		return 0;
	}
	return 1;
}

static int do_test_for_insn_bits(comedi_t *dev,unsigned int subdevice)
{
	comedi_insn insn;
	comedi_insnlist il;
	lsampl_t data[2];
	int ret;

	if(dev->subdevices[subdevice].maxdata != 1)
		return 0;

	memset(&insn,0,sizeof(insn));

	il.n_insns = 1;
	il.insns = &insn;

	insn.insn = INSN_BITS;
	insn.n = 2;
	insn.data = data;
	insn.subdev = subdevice;
	memset(data, 0, insn.n * sizeof(data[0]));

	ret = comedi_ioctl(dev->fd, COMEDI_INSNLIST, &il);

	if(ret<0 && (errno==EINVAL || errno==EIO)){
		return 0;
	}
	if(ret<0){
		perror("BUG in do_test_for_insn_bits()\n");
		return 0;
	}
	return 1;
}



