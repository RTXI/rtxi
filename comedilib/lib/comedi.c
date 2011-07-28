/*
    lib/comedi.c
    generic functions

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

INTERNAL int __comedi_init=0;

INTERNAL void initialize(void)
{
	char *s;

	__comedi_init=1;

	if( (s=getenv("COMEDILIB_LOGLEVEL")) ){
		__comedi_loglevel=strtol(s,NULL,0);
		COMEDILIB_DEBUG(3,"setting loglevel to %d\n",__comedi_loglevel);
	}
}

EXPORT_ALIAS_DEFAULT(_comedi_open,comedi_open,0.7.18);
comedi_t* _comedi_open(const char *fn)
{
	comedi_t *it;

	if(!__comedi_init)
		initialize();

	if(!(it=malloc(sizeof(comedi_t))))
		goto cleanup;
	memset(it,0,sizeof(comedi_t));

	if((it->fd=open(fn,O_RDWR))<0){
		libc_error();
		goto cleanup;
	}

	if(comedi_ioctl(it->fd, COMEDI_DEVINFO, &it->devinfo) < 0)
		goto cleanup;

	it->n_subdevices=it->devinfo.n_subdevs;

	if(get_subdevices(it) < 0)
		goto cleanup;

	it->magic=COMEDILIB_MAGIC;

	return it;
cleanup:
	if(it)
		free(it);

	return NULL;
}

EXPORT_ALIAS_DEFAULT(_comedi_close,comedi_close,0.7.18);
int _comedi_close(comedi_t *it)
{
	subdevice *s;
	int i,j;

	if(!valid_dev(it))
		return -1;
	it->magic=0;

	for(i=0;i<it->n_subdevices;i++){
		s=it->subdevices+i;
		if(s->type==COMEDI_SUBD_UNUSED)
			continue;

		if(s->subd_flags&SDF_FLAGS){
			free(s->flags_list);
		}
		if(s->subd_flags&SDF_MAXDATA){
			free(s->maxdata_list);
		}
		if(s->subd_flags&SDF_RANGETYPE){
			free(s->range_type_list);
			for(j=0;j<s->n_chan;j++)
				free(s->rangeinfo_list[j]);
			free(s->rangeinfo_list);
		}else{
			free(s->rangeinfo);
		}
		if(s->cmd_mask)free(s->cmd_mask);
		if(s->cmd_timed)free(s->cmd_timed);
	}
	if(it->subdevices){
		free(it->subdevices);
	}
	close(it->fd);
	free(it);
	return 0;
}

EXPORT_ALIAS_DEFAULT(_comedi_cancel,comedi_cancel,0.7.18);
int _comedi_cancel(comedi_t *it,unsigned int subdevice)
{
	if(!valid_dev(it)) return -1;
	return comedi_ioctl(it->fd, COMEDI_CANCEL, (void*)(unsigned long)subdevice);
}

EXPORT_ALIAS_DEFAULT(_comedi_poll,comedi_poll,0.7.18);
int _comedi_poll(comedi_t *it,unsigned int subdevice)
{
	if(!valid_dev(it)) return -1;
	return comedi_ioctl(it->fd, COMEDI_POLL, (void*)(unsigned long)subdevice);
}

EXPORT_ALIAS_DEFAULT(_comedi_fileno,comedi_fileno,0.7.18);
int _comedi_fileno(comedi_t *it)
{
	if(!valid_dev(it)) return -1;
	return it->fd;
}

EXPORT_ALIAS_DEFAULT(_comedi_trigger,comedi_trigger,0.7.18);
int _comedi_trigger(comedi_t *it,comedi_trig *t)
{
	if(!valid_dev(it) || !t)
		return -1;

	return comedi_ioctl(it->fd, COMEDI_TRIG, t);
}

EXPORT_ALIAS_DEFAULT(_comedi_command,comedi_command,0.7.18);
int _comedi_command(comedi_t *it,comedi_cmd *t)
{
	int ret;
	if(!valid_dev(it)) return -1;
	ret = comedi_ioctl(it->fd, COMEDI_CMD, t);
	__comedi_errno = errno;
	switch(__comedi_errno){
	case EIO:
		__comedi_errno = ECMDNOTSUPP;
		break;
	}
	return ret;
}

EXPORT_ALIAS_DEFAULT(_comedi_command_test,comedi_command_test,0.7.18);
int _comedi_command_test(comedi_t *it,comedi_cmd *t)
{
	int ret;
	if(!valid_dev(it)) return -1;
	ret = comedi_ioctl(it->fd, COMEDI_CMDTEST, t);
	__comedi_errno = errno;
	switch(__comedi_errno){
	case EIO:
		__comedi_errno = ECMDNOTSUPP;
		break;
	}
	return ret;
}

EXPORT_ALIAS_DEFAULT(_comedi_do_insnlist,comedi_do_insnlist,0.7.18);
int _comedi_do_insnlist(comedi_t *it,comedi_insnlist *il)
{
	int ret;
	if(!valid_dev(it)) return -1;
	ret = comedi_ioctl(it->fd, COMEDI_INSNLIST, il);
	__comedi_errno = errno;
	return ret;
}

EXPORT_ALIAS_DEFAULT(_comedi_do_insn,comedi_do_insn,0.7.18);
int _comedi_do_insn(comedi_t *it,comedi_insn *insn)
{
	if(!valid_dev(it)) return -1;
	if(it->has_insn_ioctl){
		return comedi_ioctl(it->fd, COMEDI_INSN, insn);
	}else{
		comedi_insnlist il;
		int ret;

		il.n_insns = 1;
		il.insns = insn;

		ret = comedi_ioctl(it->fd, COMEDI_INSNLIST, &il);

		if(ret<0)return ret;
		return insn->n;
	}
}

EXPORT_ALIAS_DEFAULT(_comedi_lock,comedi_lock,0.7.18);
int _comedi_lock(comedi_t *it,unsigned int subdevice)
{
	if(!valid_dev(it)) return -1;
	return comedi_ioctl(it->fd, COMEDI_LOCK, (void*)(unsigned long)subdevice);
}

EXPORT_ALIAS_DEFAULT(_comedi_unlock,comedi_unlock,0.7.18);
int _comedi_unlock(comedi_t *it,unsigned int subdevice)
{
	if(!valid_dev(it)) return -1;
	return comedi_ioctl(it->fd, COMEDI_UNLOCK, (void*)(unsigned long)subdevice);
}

