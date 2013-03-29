/*
    lib/error.c
    error functions and data

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
#include <string.h>

char *__comedilib_error_strings[]={
	_s("No error"),
	_s("Unknown error"),
	_s("Bad comedi_t structure"),
	_s("Invalid subdevice"),
	_s("Invalid channel"),
	_s("Buffer overflow"),
	_s("Buffer underflow"),
	_s("Command not supported"),
	_s("Not supported"),
};
#define n_errors (sizeof(__comedilib_error_strings)/sizeof(void *))

int __comedi_loglevel=1;
TLS int __comedi_errno=0;

EXPORT_ALIAS_DEFAULT(_comedi_loglevel,comedi_loglevel,0.7.18);
int _comedi_loglevel(int loglevel)
{
	int old_loglevel=__comedi_loglevel;
	
	__comedi_loglevel=loglevel;
	
	return old_loglevel;
}

EXPORT_ALIAS_DEFAULT(_comedi_errno,comedi_errno,0.7.18);
int _comedi_errno(void)
{
	return __comedi_errno;
}

EXPORT_ALIAS_DEFAULT(_comedi_strerror,comedi_strerror,0.7.18);
const char* _comedi_strerror(int errnum)
{
	if(errnum<COMEDI_NOERROR || errnum>=COMEDI_NOERROR+n_errors)
		return strerror(errnum);

	return GETTEXT(__comedilib_error_strings[errnum-COMEDI_NOERROR]);
}

EXPORT_ALIAS_DEFAULT(_comedi_perror,comedi_perror,0.7.18);
void _comedi_perror(const char *s)
{
	if(__comedi_loglevel>=3){
		fprintf(stderr,"comedi_perror(): __comedi_errno=%d\n",__comedi_errno);
	}
	if(!s)s="comedilib";
	fprintf(stderr,"%s: %s\n",s,comedi_strerror(__comedi_errno));
}

void libc_error(void)
{
	__comedi_errno=errno;
	if(__comedi_loglevel>=2){
		comedi_perror("libc error");
	}
}

void internal_error(int err)
{
	__comedi_errno=err;
	if(__comedi_loglevel>=2){
		comedi_perror("internal error");
	}
}



int valid_dev(comedi_t *it)
{
	if(!it || it->magic!=COMEDILIB_MAGIC){
		internal_error(EBAD_CT);
		return 0;
	}
	
	return 1;
}

int valid_subd(comedi_t *it,unsigned int subd)
{
	if(!valid_dev(it))return 0;
	if(subd>=it->n_subdevices){
		internal_error(EINVAL_SUBD);
		return 0;
	}
	
	return 1;
}

int valid_chan(comedi_t *it,unsigned int subd,unsigned int chan)
{
	if(!valid_subd(it,subd))return 0;
	if(chan>=it->subdevices[subd].n_chan){
		internal_error(EINVAL_CHAN);
		return 0;
	}
	
	return 1;
}


