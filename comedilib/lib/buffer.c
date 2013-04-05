/*
    lib/buffer.c
    functions for manipulating buffers

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
#include <string.h>

EXPORT_ALIAS_DEFAULT(_comedi_set_buffer_size,comedi_set_buffer_size,0.7.18);
int _comedi_set_buffer_size(comedi_t *it, unsigned int subdev, unsigned int size)
{
	int ret;
	comedi_bufconfig bc;

	if(!valid_subd(it,subdev)) return -1;
	memset(&bc, 0, sizeof(bc));
	bc.subdevice = subdev;
	bc.size = size;
	ret = comedi_ioctl(it->fd, COMEDI_BUFCONFIG, &bc);
	if(ret < 0) return ret;

	return bc.size;
}

EXPORT_ALIAS_DEFAULT(_comedi_set_max_buffer_size,comedi_set_max_buffer_size,0.7.18);
int _comedi_set_max_buffer_size(comedi_t *it, unsigned int subdev, unsigned int max_size)
{
	int ret;
	comedi_bufconfig bc;

	if(!valid_subd(it,subdev)) return -1;
	memset(&bc, 0, sizeof(bc));
	bc.subdevice = subdev;
	bc.maximum_size = max_size;
	ret = comedi_ioctl(it->fd, COMEDI_BUFCONFIG, &bc);
	if(ret < 0) return ret;

	return bc.maximum_size;
}

EXPORT_ALIAS_DEFAULT(_comedi_get_max_buffer_size,comedi_get_max_buffer_size,0.7.18);
int _comedi_get_max_buffer_size(comedi_t *it, unsigned int subdevice)
{
	return comedi_set_max_buffer_size(it, subdevice, 0);
}

EXPORT_ALIAS_DEFAULT(_comedi_get_buffer_size,comedi_get_buffer_size,0.7.18);
int _comedi_get_buffer_size(comedi_t *it, unsigned int subdev)
{
	return comedi_set_buffer_size(it, subdev, 0);
}

EXPORT_ALIAS_DEFAULT(_comedi_get_buffer_contents,comedi_get_buffer_contents,0.7.18);
int _comedi_get_buffer_contents(comedi_t *it, unsigned int subdev)
{
	int ret;
	comedi_bufinfo bi;

	if(!valid_subd(it,subdev)) return -1;
	memset(&bi, 0, sizeof(bi));
	bi.subdevice = subdev;
	ret = comedi_ioctl(it->fd, COMEDI_BUFINFO, &bi);
	if(ret < 0)
	{
		if(__comedi_errno == EPIPE)__comedi_errno = EBUF_OVR;
		return ret;
	}
	return bi.buf_write_count - bi.buf_read_count;
}

EXPORT_ALIAS_DEFAULT(_comedi_mark_buffer_read,comedi_mark_buffer_read,0.7.18);
int _comedi_mark_buffer_read(comedi_t *it, unsigned int subdev, unsigned int bytes)
{
	int ret;
	comedi_bufinfo bi;

	if(!valid_subd(it,subdev)) return -1;
	memset(&bi, 0, sizeof(bi));
	bi.subdevice = subdev;
	bi.bytes_read = bytes;
	ret = comedi_ioctl(it->fd, COMEDI_BUFINFO, &bi);
	if(ret < 0)
	{
		if(__comedi_errno == EPIPE)__comedi_errno = EBUF_OVR;
		return -1;
	}
	return bi.bytes_read;
}

EXPORT_ALIAS_DEFAULT(_comedi_mark_buffer_written,comedi_mark_buffer_written,0.8.0);
int _comedi_mark_buffer_written(comedi_t *it, unsigned int subdev, unsigned int bytes)
{
	int ret;
	comedi_bufinfo bi;

	if(!valid_subd(it,subdev)) return -1;
	memset(&bi, 0, sizeof(bi));
	bi.subdevice = subdev;
	bi.bytes_written = bytes;
	ret = comedi_ioctl(it->fd, COMEDI_BUFINFO, &bi);
	if(ret < 0)
	{
		if(__comedi_errno == EPIPE)__comedi_errno = EBUF_UNDR;
		return -1;
	}
	return bi.bytes_written;
}

EXPORT_ALIAS_DEFAULT(_comedi_get_buffer_offset,comedi_get_buffer_offset,0.7.18);
int _comedi_get_buffer_offset(comedi_t *it, unsigned int subdev)
{
	int ret;
	comedi_bufinfo bi;

	if(!valid_subd(it,subdev)) return -1;
	memset(&bi, 0, sizeof(bi));
	bi.subdevice = subdev;
	ret = comedi_ioctl(it->fd, COMEDI_BUFINFO, &bi);
	if(ret < 0) return ret;
	return bi.buf_read_ptr;
}

EXPORT_ALIAS_DEFAULT(_comedi_get_front_count,comedi_get_front_count,0.7.18);
int _comedi_get_front_count(comedi_t *it, unsigned int subdev)
{
	int ret;
	comedi_bufinfo bi;

	if(!valid_subd(it,subdev)) return -1;
	memset(&bi, 0, sizeof(bi));
	bi.subdevice = subdev;
	ret = comedi_ioctl(it->fd, COMEDI_BUFINFO, &bi);
	if(ret < 0) return ret;
	return bi.buf_write_count;
}

