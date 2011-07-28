/*
 * Copyright (c) 1999 Joseph E. Smith <jes@presto.med.upenn.edu>
 *
 * All rights reserved.  This program is free software.  You may
 * redistribute it and/or modify it under the same terms as Perl itself.
 */

#ifdef __cplusplus
extern "C" {
#endif
#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"
#ifdef __cplusplus
}
#endif

#include <comedi.h>
#include <comedilib.h>

static int
not_here(s)
char *s;
{
    croak("%s not implemented on this architecture", s);
    return -1;
}

static double
constant(name, arg)
char *name;
int arg;
{
    errno = 0;
    switch (*name) {
    }
    errno = EINVAL;
    return 0;

not_there:
    errno = ENOENT;
    return 0;
}

typedef struct comedi_trig_struct Trigger;

MODULE = Comedi::Lib		PACKAGE = Comedi::Lib		


double
constant(name,arg)
	char *		name
	int		arg

comedi_t *
comedi_open(fn)
	const char *	fn

void
comedi_close(it)
	comedi_t *	it

int
comedi_loglevel(level)
	int	level

void
comedi_perror(s)
	const char *	s

char *
comedi_strerror(n)
	int	n

int
comedi_errno()

int
comedi_fileno(it)
	comedi_t *	it

#/* queries */

int
comedi_get_n_subdevices(it)
	comedi_t *	it

int
comedi_get_version_code(it)
	comedi_t *	it

char *
comedi_get_driver_name(it)
	comedi_t *	it

char *
comedi_get_board_name(it)
	comedi_t *	it

int
comedi_get_subdevice_type(it, subdevice)
	comedi_t *	it
	unsigned int	subdevice

int
comedi_find_subdevice_by_type(it, type, subd)
	comedi_t *	it
	int	type
	unsigned int	subd

int
comedi_get_n_channels(it, subdevice)
	comedi_t *	it
	unsigned int	subdevice

lsampl_t
comedi_get_maxdata(it, subdevice, chan)
	comedi_t *	it
	unsigned int	subdevice
	unsigned int	chan

int
comedi_get_rangetype(it, subdevice, chan)
	comedi_t *	it
	unsigned int	subdevice
	unsigned int	chan

int
comedi_find_range(it, subd, chan, unit, min, max)
	comedi_t *	it
	unsigned int	subd
	unsigned int	chan
	unsigned int	unit
	double	min
	double	max

#/* triggers and commands */

int
comedi_cancel(it, subd)
	comedi_t *	it
	unsigned int	subd

#/*
# * This function takes a scalar holding a packed comedi trigger structure.
# * The 'comedi_trigger' function (found in Lib.pm) takes a Perl trigger
# * object and passes a packed structure to this function.
# */

int
_comedi_trigger(it, trig)
	comedi_t *	it
	comedi_trig *	trig
	PREINIT:
	  int i;
	CODE:
#ifdef LIB_DEBUG
	  fprintf(stderr, "trigger: it=%08x, trig=%08x\n", it, trig);
	  fprintf(stderr, "trigger: mode=%d, n=%d, tv=(%d,%d), nch=%d\n",
	    trig->mode, trig->n, trig->trigvar, trig->trigvar1, trig->n_chan);
	  fprintf(stderr, "chan list @%08x:", trig->chanlist);
	  for(i = 0; i < trig->n_chan; ++i) {
	    fprintf(stderr, " %08x", trig->chanlist[i]);
	  }
	  fprintf(stderr, "\n");
	  fprintf(stderr, "data buffer @%08x:", trig->data);
	  for(i = 0; i < 6; ++i) {
	    fprintf(stderr, " %04x", trig->data[i]);
	  }
	  fprintf(stderr, "\n");
#endif
	  RETVAL = comedi_trigger(it, trig);
	OUTPUT:
	  RETVAL

int
comedi_command(it, cmd)
	comedi_t *	it
	comedi_cmd *	cmd

#/* physical units */

double
comedi_to_phys(data, rng, maxdata)
	lsampl_t	data
	comedi_range *	rng
	lsampl_t	maxdata

lsampl_t
comedi_from_phys(data, rng, maxdata)
	double	data
	comedi_range *	rng
	lsampl_t	maxdata

#/* synchronous stuff */

int
comedi_data_read(it, subd, chan, range, aref, data)
	comedi_t *	it
	unsigned int	subd
	unsigned int	chan
	unsigned int	range
	unsigned int	aref
	lsampl_t *	data


int
comedi_data_write(it, subd, chan, range, aref, data)
	comedi_t *	it
	unsigned int	subd
	unsigned int	chan
	unsigned int	range
	unsigned int	aref
	lsampl_t	data

#/* slowly varying stuff */

int
comedi_sv_init(it, dev, subd, chan)
	comedi_sv_t *	it
	comedi_t *	dev
	unsigned int	subd
	unsigned int	chan
	OUTPUT:
	it

int
comedi_sv_update(it)
	comedi_sv_t *	it

int
comedi_sv_measure(it, data)
	comedi_sv_t *	it
	double *	data

#/* dio config */

int
comedi_dio_config(it, subd, chan, dir)
	comedi_t *	it
	unsigned int	subd
	unsigned int	chan
	unsigned int	dir

int
comedi_dio_read(it, subd, chan, bit)
	comedi_t *	it
	unsigned int	subd
	unsigned int	chan
	unsigned int	&bit
	OUTPUT:
	bit

int
comedi_dio_write(it, subd, chan, bit)
	comedi_t *	it
	unsigned int	subd
	unsigned int	chan
	unsigned int	bit

int
comedi_dio_bitfield(it,	subd, write_mask, bits)
	comedi_t *	it
	unsigned int	subd
	unsigned int	write_mask
	unsigned int 	&bits
	OUTPUT:
	bits

#/* timer stuff */

int
comedi_get_timer(it, subdev, freq, trigvar, actual_freq)
	comedi_t *	it
	unsigned int	subdev
	double	freq
	unsigned int	&trigvar
	double	&actual_freq
	OUTPUT:
	trigvar
	actual_freq

int
comedi_timed_1chan(it, subdev, chan, range, aref, freq, n_samples, data)
	comedi_t *	it
	unsigned int	subdev
	unsigned int	chan
	unsigned int	range
	unsigned int	aref
	double	freq
	unsigned int	n_samples
	double *	data
	CODE:
#ifdef LIB_DEBUG
	  fprintf(stderr, "timed_1chan: CR=(%d,%d,%d), f=%g, n=%d, data->0x%08x\n",
	    chan, range, aref, freq, n_samples, data);
#endif
	  RETVAL = comedi_timed_1chan(it, subdev, chan, range, aref, freq, n_samples, data);
	OUTPUT:
	  RETVAL



#/* range stuff */

comedi_range *
comedi_get_range(it, subdevice, chan, range)
	comedi_t *	it
	unsigned int	subdevice
	unsigned int	chan
	unsigned int	range

