/***********************************************************
           Interface file for wrapping Comedilib

    Copyright (C) 2003 Bryan Cole  <bryan.cole@teraview.co.uk>
    Copyright (C) 1998-2002 David A. Schleef <ds@schleef.org>
    Copyright (C) 2003,2004 Frank Mori Hess <fmhess@users.sourceforge.net>
    Copyright (C) 2003 Steven Jenkins <steven.jenkins@ieee.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

***********************************************************
*
*     This file was created with Python wrappers in mind but wil
*  probably work for other swig-supported script languages
*
*    to regenerate the wrappers run:
*    swig -python comedi.i
*
***********************************************************/
%module comedi
%{
#include "comedilib.h"
%}
%include "carrays.i"
%include "typemaps.i"

%inline %{
static unsigned int cr_pack(unsigned int chan, unsigned int rng, unsigned int aref){
	return CR_PACK(chan,rng,aref);
}
static unsigned int cr_pack_flags(unsigned int chan, unsigned int rng, unsigned int aref, unsigned int flags){
	return CR_PACK_FLAGS(chan,rng,aref, flags);
}
static unsigned int cr_chan(unsigned int a){
	return CR_CHAN(a);
}
static unsigned int cr_range(unsigned int a){
	return CR_RANGE(a);
}
static unsigned int cr_aref(unsigned int a){
	return CR_AREF(a);
}
%}

%array_class(unsigned int, chanlist);

%typemap(ruby, argout) comedi_cmd *INOUT(VALUE info) {
    $result = output_helper($result, $arg);
};

%include "comedi.h"
%include "comedilib.h"

