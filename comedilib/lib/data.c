/*
    lib/data.c
    functions for reading/writing data

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


EXPORT_ALIAS_DEFAULT(_comedi_data_write,comedi_data_write,0.7.18);
int _comedi_data_write(comedi_t *it,unsigned int subdev,unsigned int chan,unsigned int range,
		unsigned int aref,lsampl_t data)
{
	subdevice *s;

	if(!valid_chan(it,subdev,chan))
		return -1;

	s=it->subdevices+subdev;

	if(it->has_insnlist_ioctl){
		comedi_insn insn;

		memset(&insn,0,sizeof(insn));

		insn.insn = INSN_WRITE;
		insn.n = 1;
		insn.data = &data;
		insn.subdev = subdev;
		insn.chanspec = CR_PACK(chan,range,aref);

		return comedi_do_insn(it,&insn);
	}else{
		comedi_trig cmd={
			mode:		0,
			flags:		TRIG_WRITE,
			n_chan:		1,
			n:		1,
			trigsrc:	0,
			trigvar:	0,
			trigvar1:	0,
		};
		sampl_t sdata[2];

		sdata[0]=data & 0xffff;
		sdata[1]=(data >> 16) & 0xffff;
		chan=CR_PACK(chan,range,aref);

		cmd.subdev=subdev;
		if(it->subdevices[subdev].subd_flags & SDF_LSAMPL){
			cmd.data=sdata;
		}else{
			cmd.data=sdata;
		}
		cmd.chanlist=&chan;

		return comedi_ioctl(it->fd, COMEDI_TRIG, &cmd);
	}
}

static int comedi_internal_data_read_n(comedi_t *it,
	unsigned int subdev, unsigned int chanspec, lsampl_t *data,
	unsigned int n)
{
	subdevice *s;

	if(!valid_subd(it,subdev))
		return -1;
	if(n == 0) return 0;

	s = it->subdevices + subdev;

	if(it->has_insnlist_ioctl){
		comedi_insn insn;

		memset(&insn,0,sizeof(insn));

		insn.insn = INSN_READ;
		insn.n = n;
		insn.data = data;
		insn.subdev = subdev;
		insn.chanspec = chanspec;
		memset(insn.data, 0, n * sizeof(data[0]));	// for valgrind
		return comedi_do_insn(it,&insn);
	}else{
		comedi_trig cmd={
			mode:		0,
			flags:		0,
			n_chan:		1,
			n:		n,
			trigsrc:	0,
			trigvar:	0,
			trigvar1:	0,
		};
		int ret;
		sampl_t sdata[n];
		unsigned int i;

		cmd.subdev=subdev;
		cmd.chanlist=&chanspec;
		if(s->subd_flags & SDF_LSAMPL){
			cmd.data=(sampl_t *)data;
		}else{
			cmd.data=sdata;
		}

		ret = comedi_ioctl(it->fd, COMEDI_TRIG, &cmd);
		if(ret<0)
			return ret;

		if(!(s->subd_flags & SDF_LSAMPL)){
			for( i = 0; i < n; i++)
				data[i] = sdata[i];
		}

		return 0;
	}
}

EXPORT_ALIAS_DEFAULT(_comedi_data_read_n,comedi_data_read_n,0.7.18);
int _comedi_data_read_n(comedi_t *it, unsigned int subdev,
	unsigned int chan, unsigned int range,
	unsigned int aref, lsampl_t *data, unsigned int n)
{
	static const int max_chunk_size = 100;
	unsigned int chunk_size;
	unsigned int sample_count = 0;
	int retval;

	while( n )
	{
		if( n > max_chunk_size)
			chunk_size = max_chunk_size;
		else
			chunk_size = n;
		retval = comedi_internal_data_read_n(it, subdev,
			CR_PACK(chan, range, aref),
			data+sample_count, chunk_size);
		if( retval < 0 ) return retval;
		n -= chunk_size;
		sample_count += chunk_size;
	}
	return 0;
}

EXPORT_ALIAS_DEFAULT(_comedi_data_read,comedi_data_read,0.7.18);
int _comedi_data_read(comedi_t *it, unsigned int subdev, unsigned int chan,
	unsigned int range, unsigned int aref, lsampl_t *data)
{
	return comedi_internal_data_read_n(it, subdev,
		CR_PACK(chan, range, aref), data, 1);
}

EXPORT_ALIAS_DEFAULT(_comedi_data_read_hint,comedi_data_read_hint,0.7.19);
int _comedi_data_read_hint(comedi_t *it, unsigned int subdev,
	unsigned int chan, unsigned int range, unsigned int aref)
{
	lsampl_t dummy_data;
	return comedi_internal_data_read_n(it, subdev,
		CR_PACK(chan, range, aref), &dummy_data, 0);
}

EXPORT_ALIAS_DEFAULT(_comedi_data_read_delayed,comedi_data_read_delayed,0.7.19);
int _comedi_data_read_delayed( comedi_t *it, unsigned int subdev,
	unsigned int chan, unsigned int range, unsigned int aref,
	lsampl_t *data, unsigned int nano_sec)
{
	subdevice *s;
	comedi_insnlist ilist;
	comedi_insn insn[3];
	lsampl_t delay = nano_sec;

	if( !valid_chan( it, subdev, chan ) )
		return -1;

	s = it->subdevices + subdev;

	memset( insn, 0, sizeof(insn) );
	memset( &ilist, 0, sizeof(ilist) );
	memset(data, 0, sizeof(*data));	// for valgrind

	// setup, no conversions
	insn[0].insn = INSN_READ;
	insn[0].n = 0;
	insn[0].data = data;
	insn[0].subdev = subdev;
	insn[0].chanspec = CR_PACK( chan, range, aref );
	// delay
	insn[1].insn = INSN_WAIT;
	insn[1].n = 1;
	insn[1].data = &delay;
	// take conversion
	insn[2].insn = INSN_READ;
	insn[2].n = 1;
	insn[2].data = data;
	insn[2].subdev = subdev;
	insn[2].chanspec = CR_PACK( chan, range, aref );

	ilist.insns = insn;
	ilist.n_insns = sizeof(insn) / sizeof(insn[0]);

	return comedi_do_insnlist(it, &ilist);
}

