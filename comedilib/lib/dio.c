/*
    lib/dio.c
    digital I/O routines

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
#include <errno.h>
#include <string.h>

#include "libinternal.h"


EXPORT_ALIAS_DEFAULT(_comedi_dio_config,comedi_dio_config,0.7.18);
int _comedi_dio_config(comedi_t *it, unsigned int subdev, unsigned int chan, unsigned int io)
{
	subdevice *s;
	enum configuration_ids config_id;

	if(!valid_chan(it,subdev,chan))
		return -1;

	s=it->subdevices+subdev;
	if(s->type!=COMEDI_SUBD_DIO){
		internal_error(EINVAL_SUBD);
		return -1;
	}
	switch(io)
	{
	case COMEDI_INPUT:
		config_id = INSN_CONFIG_DIO_INPUT;
		break;
	case COMEDI_OUTPUT:
		config_id = INSN_CONFIG_DIO_OUTPUT;
		break;
	default:
		internal_error(EINVAL);
		return -1;
	}
	if(it->has_insnlist_ioctl){
		comedi_insn insn;
		lsampl_t data = config_id;

		memset(&insn,0,sizeof(insn));
		insn.insn = INSN_CONFIG;
		insn.n = 1;
		insn.data = &data;
		insn.subdev = subdev;
		insn.chanspec = CR_PACK(chan,0,0);

		if(comedi_do_insn(it,&insn) < 0) return -1;
		else return 0;
	}else
	{
		comedi_trig trig;
		sampl_t data = config_id;

		memset(&trig,0,sizeof(trig));
		trig.flags=TRIG_CONFIG|TRIG_WRITE;
		trig.n_chan=1;
		trig.n=1;
		trig.subdev=subdev;
		trig.chanlist=&chan;
		trig.data=&data;

		if(comedi_trigger(it,&trig) < 0) return -1;
		else return 0;
	}
}

EXPORT_ALIAS_DEFAULT(_comedi_dio_get_config, comedi_dio_get_config, 0.8.0);
int _comedi_dio_get_config(comedi_t *it,unsigned int subdev, unsigned int chan, unsigned int *io)
{
	subdevice *s;
	comedi_insn insn;
	lsampl_t data[2];
	int retval;

	if(!valid_chan(it,subdev,chan))
		return -1;

	s=it->subdevices+subdev;
	if(s->type!=COMEDI_SUBD_DIO)
		return -1;

	if(it->has_insnlist_ioctl == 0)
		return -1;

	memset(&insn,0,sizeof(insn));
	insn.insn = INSN_CONFIG;
	insn.n = sizeof(data) / sizeof(data[0]);
	insn.data = data;
	insn.subdev = subdev;
	insn.chanspec = CR_PACK(chan,0,0);
	memset(data, 0, insn.n * sizeof(*data));
	data[0] = INSN_CONFIG_DIO_QUERY;

	retval = comedi_do_insn(it,&insn);
	if(retval < 0) return retval;
	*io = data[1];
	return 0;
}

EXPORT_ALIAS_DEFAULT(_comedi_dio_read,comedi_dio_read,0.7.18);
int _comedi_dio_read(comedi_t *it,unsigned int subdev,unsigned int chan,
	unsigned int *val)
{
	subdevice *s;
	int ret;

	if(!valid_chan(it,subdev,chan))
		return -1;

	s = it->subdevices+subdev;
	if(s->type!=COMEDI_SUBD_DIO &&
	   s->type!=COMEDI_SUBD_DO &&
	   s->type!=COMEDI_SUBD_DI)
		return -1;

	if(it->has_insnlist_ioctl){
		comedi_insn insn;
		lsampl_t data = 0;

		memset(&insn,0,sizeof(insn));
		insn.insn = INSN_READ;
		insn.n = 1;
		insn.data = &data;
		insn.subdev = subdev;
		insn.chanspec = CR_PACK(chan,0,0);

		ret = comedi_do_insn(it,&insn);

		*val = data;

		return ret;
	}else{
		comedi_trig trig;
		sampl_t data;

		memset(&trig,0,sizeof(trig));
		trig.n_chan=1;
		trig.n=1;
		trig.subdev=subdev;
		trig.chanlist=&chan;
		trig.data=&data;

		ret=comedi_trigger(it,&trig);

		if(ret>=0 && val)*val=data;

		return ret;
	}
}

EXPORT_ALIAS_DEFAULT(_comedi_dio_write,comedi_dio_write,0.7.18);
int _comedi_dio_write(comedi_t *it,unsigned int subdev,unsigned int chan,
	unsigned int val)
{
	subdevice *s;

	if(!valid_chan(it,subdev,chan))
		return -1;

	s = it->subdevices+subdev;
	if(s->type!=COMEDI_SUBD_DIO &&
	   s->type!=COMEDI_SUBD_DO)
		return -1;

	if(it->has_insnlist_ioctl){
		comedi_insn insn;
		lsampl_t data;

		memset(&insn,0,sizeof(insn));
		insn.insn = INSN_WRITE;
		insn.n = 1;
		insn.data = &data;
		insn.subdev = subdev;
		insn.chanspec = CR_PACK(chan,0,0);

		data = val;

		return comedi_do_insn(it,&insn);
	}else{
		comedi_trig trig;
		sampl_t data;

		data=val;

		memset(&trig,0,sizeof(trig));
		trig.n_chan=1;
		trig.n=1;
		trig.flags=TRIG_WRITE;
		trig.subdev=subdev;
		trig.chanlist=&chan;
		trig.data=&data;

		return comedi_trigger(it,&trig);
	}
}

EXPORT_ALIAS_DEFAULT(_comedi_dio_bitfield2,comedi_dio_bitfield2,0.8.0);
int _comedi_dio_bitfield2(comedi_t *it, unsigned int subdev, unsigned int mask, unsigned int *bits, unsigned base_channel)
{
	int ret;
	unsigned int m,bit;
	subdevice *s;

	if(!valid_subd(it, subdev))
		return -1;

	s = it->subdevices + subdev;

	if(s->type != COMEDI_SUBD_DIO && s->type != COMEDI_SUBD_DO &&
		s->type != COMEDI_SUBD_DI)
		return -1;

	if(s->has_insn_bits)
	{
		comedi_insn insn;
		lsampl_t data[2];

		memset(&insn,0,sizeof(insn));

		insn.insn = INSN_BITS;
		insn.chanspec = base_channel;
		insn.n = 2;
		insn.data = data;
		insn.subdev = subdev;

		data[0] = mask;
		data[1] = *bits;

		ret = comedi_do_insn(it, &insn);

		if(ret < 0) return ret;

		*bits = data[1];

		return 0;
	}else{
		unsigned i;
		for(i = 0, m = 1; i < 32; ++i, m <<= 1)
		{
			if(mask & m)
			{
				bit = (*bits & m) ? 1 : 0;
				ret = comedi_dio_write(it, subdev, base_channel + i, bit);
			}else{
				ret = comedi_dio_read(it, subdev, base_channel + i, &bit);
				if(bit) *bits |= m;
				else (*bits) &= ~m;
			}
			if(ret < 0) return ret;
		}
		return 0;
	}
}

EXPORT_ALIAS_DEFAULT(_comedi_dio_bitfield,comedi_dio_bitfield,0.7.18);
int _comedi_dio_bitfield(comedi_t *it, unsigned int subdev, unsigned int mask, unsigned int *bits)
{
	return _comedi_dio_bitfield2(it, subdev, mask, bits, 0);
}

