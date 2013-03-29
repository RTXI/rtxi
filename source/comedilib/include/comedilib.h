/*
    include/comedilib.h
    header file for the comedi library routines

    COMEDI - Linux Control and Measurement Device Interface
    Copyright (C) 1998-2002 David A. Schleef <ds@schleef.org>

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

*/


#ifndef _COMEDILIB_H
#define _COMEDILIB_H

#include <comedi.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Macros for swig to %include this file. */
#ifdef SWIG
#define SWIG_OUTPUT(x)  OUTPUT
#define SWIG_INPUT(x)   INPUT
#define SWIG_INOUT(x)   INOUT
#else
#define SWIG_OUTPUT(x)  x
#define SWIG_INPUT(x)   x
#define SWIG_INOUT(x)   x
#endif

typedef struct comedi_t_struct comedi_t;

typedef struct{
	double min;
	double max;
	unsigned int unit;
}comedi_range;

typedef struct comedi_sv_struct{
	comedi_t *dev;
	unsigned int subdevice;
	unsigned int chan;

	/* range policy */
	int range;
	int aref;

	/* number of measurements to average (for ai) */
	int n;

	lsampl_t maxdata;
}comedi_sv_t;

enum comedi_oor_behavior {
	COMEDI_OOR_NUMBER = 0,
	COMEDI_OOR_NAN
};




comedi_t *comedi_open(const char *fn);
int comedi_close(comedi_t *it);

/* logging */
int comedi_loglevel(int loglevel);
void comedi_perror(const char *s);
const char *comedi_strerror(int errnum);
int comedi_errno(void);
int comedi_fileno(comedi_t *it);

/* global behavior */
enum comedi_oor_behavior comedi_set_global_oor_behavior(enum comedi_oor_behavior behavior);

/* device queries */
int comedi_get_n_subdevices(comedi_t *it);
#define COMEDI_VERSION_CODE(a,b,c) (((a)<<16) | ((b)<<8) | (c))
int comedi_get_version_code(comedi_t *it);
const char *comedi_get_driver_name(comedi_t *it);
const char *comedi_get_board_name(comedi_t *it);
int comedi_get_read_subdevice(comedi_t *dev);
int comedi_get_write_subdevice(comedi_t *dev);

/* subdevice queries */
int comedi_get_subdevice_type(comedi_t *it,unsigned int subdevice);
int comedi_find_subdevice_by_type(comedi_t *it,int type,unsigned int subd);
int comedi_get_subdevice_flags(comedi_t *it,unsigned int subdevice);
int comedi_get_n_channels(comedi_t *it,unsigned int subdevice);
int comedi_range_is_chan_specific(comedi_t *it,unsigned int subdevice);
int comedi_maxdata_is_chan_specific(comedi_t *it,unsigned int subdevice);

/* channel queries */
lsampl_t comedi_get_maxdata(comedi_t *it,unsigned int subdevice,
	unsigned int chan);
int comedi_get_n_ranges(comedi_t *it,unsigned int subdevice,
	unsigned int chan);
comedi_range * comedi_get_range(comedi_t *it,unsigned int subdevice,
	unsigned int chan,unsigned int range);
int comedi_find_range(comedi_t *it,unsigned int subd,unsigned int chan,
	unsigned int unit,double min,double max);

/* buffer queries */
int comedi_get_buffer_size(comedi_t *it,unsigned int subdevice);
int comedi_get_max_buffer_size(comedi_t *it,unsigned int subdevice);
int comedi_set_buffer_size(comedi_t *it,unsigned int subdevice,
	unsigned int len);

/* low-level stuff */
#ifdef _COMEDILIB_DEPRECATED
int comedi_trigger(comedi_t *it,comedi_trig *trig); /* deprecated */
#endif
int comedi_do_insnlist(comedi_t *it,comedi_insnlist *il);
int comedi_do_insn(comedi_t *it,comedi_insn *insn);
int comedi_lock(comedi_t *it,unsigned int subdevice);
int comedi_unlock(comedi_t *it,unsigned int subdevice);

/* physical units */
double comedi_to_phys(lsampl_t data,comedi_range *rng,lsampl_t maxdata);
lsampl_t comedi_from_phys(double data,comedi_range *rng,lsampl_t maxdata);
int comedi_sampl_to_phys(double *dest, int dst_stride, sampl_t *src,
	int src_stride, comedi_range *rng, lsampl_t maxdata, int n);
int comedi_sampl_from_phys(sampl_t *dest,int dst_stride,double *src,
	int src_stride, comedi_range *rng, lsampl_t maxdata, int n);

/* syncronous stuff */
int comedi_data_read(comedi_t *it,unsigned int subd,unsigned int chan,
	unsigned int range,unsigned int aref,lsampl_t *SWIG_OUTPUT(data));
int comedi_data_read_n(comedi_t *it,unsigned int subd,unsigned int chan,
	unsigned int range,unsigned int aref,lsampl_t *SWIG_OUTPUT(data), unsigned int n);
int comedi_data_read_hint(comedi_t *it,unsigned int subd,unsigned int chan,
	unsigned int range,unsigned int aref);
int comedi_data_read_delayed(comedi_t *it,unsigned int subd,unsigned int chan,
	unsigned int range,unsigned int aref,lsampl_t *SWIG_OUTPUT(data), unsigned int nano_sec);
int comedi_data_write(comedi_t *it,unsigned int subd,unsigned int chan,
	unsigned int range,unsigned int aref,lsampl_t data);
int comedi_dio_config(comedi_t *it,unsigned int subd,unsigned int chan,
	unsigned int dir);
int comedi_dio_get_config(comedi_t *it,unsigned int subd,unsigned int chan,
	unsigned int *SWIG_OUTPUT(dir));
int comedi_dio_read(comedi_t *it,unsigned int subd,unsigned int chan,
	unsigned int *SWIG_OUTPUT(bit));
int comedi_dio_write(comedi_t *it,unsigned int subd,unsigned int chan,
	unsigned int bit);
int comedi_dio_bitfield2(comedi_t *it,unsigned int subd,
	unsigned int write_mask, unsigned int *SWIG_INOUT(bits), unsigned int base_channel);
/* Should be moved to _COMEDILIB_DEPRECATED once bindings for other languages are updated
 * to use comedi_dio_bitfield2() instead.*/
int comedi_dio_bitfield(comedi_t *it,unsigned int subd,
	unsigned int write_mask, unsigned int *SWIG_INOUT(bits));

/* slowly varying stuff */
int comedi_sv_init(comedi_sv_t *it,comedi_t *dev,unsigned int subd,unsigned int chan);
int comedi_sv_update(comedi_sv_t *it);
int comedi_sv_measure(comedi_sv_t *it,double *data);

/* streaming I/O (commands) */

int comedi_get_cmd_src_mask(comedi_t *dev,unsigned int subdevice,
	comedi_cmd *SWIG_INOUT(cmd));
int comedi_get_cmd_generic_timed(comedi_t *dev,unsigned int subdevice,
	comedi_cmd *SWIG_INOUT(cmd), unsigned chanlist_len, unsigned scan_period_ns);
int comedi_cancel(comedi_t *it,unsigned int subdevice);
int comedi_command(comedi_t *it,comedi_cmd *cmd);
int comedi_command_test(comedi_t *it,comedi_cmd *SWIG_INOUT(cmd));
int comedi_poll(comedi_t *dev,unsigned int subdevice);

/* buffer control */

int comedi_set_max_buffer_size(comedi_t *it, unsigned int subdev,
	unsigned int max_size);
int comedi_get_buffer_contents(comedi_t *it, unsigned int subdev);
int comedi_mark_buffer_read(comedi_t *it, unsigned int subdev,
	unsigned int bytes);
int comedi_mark_buffer_written(comedi_t *it, unsigned int subdev,
	unsigned int bytes);
int comedi_get_buffer_offset(comedi_t *it, unsigned int subdev);

#ifdef _COMEDILIB_DEPRECATED
/*
 * The following functions are deprecated and should not be used.
 */
int comedi_get_timer(comedi_t *it,unsigned int subdev,double freq,
	unsigned int *trigvar,double *actual_freq);
int comedi_timed_1chan(comedi_t *it,unsigned int subdev,unsigned int chan,
	unsigned int range, unsigned int aref,double freq,
	unsigned int n_samples,double *data);
int comedi_get_rangetype(comedi_t *it,unsigned int subdevice,
	unsigned int chan);
#endif


#ifndef _COMEDILIB_STRICT_ABI
/*
   The following prototypes are _NOT_ part of the Comedilib ABI, and
   may change in future versions without regard to source or binary
   compatibility.  In practice, this is a holding place for the next
   library ABI version change.
 */
/* structs and functions used for parsing calibration files */
typedef struct
{
	unsigned int subdevice;
	unsigned int channel;
	unsigned int value;
} comedi_caldac_t;
#define COMEDI_MAX_NUM_POLYNOMIAL_COEFFICIENTS 4
typedef struct
{
	double coefficients[COMEDI_MAX_NUM_POLYNOMIAL_COEFFICIENTS];
	double expansion_origin;
	unsigned order;
} comedi_polynomial_t;
typedef struct
{
	comedi_polynomial_t *to_phys;
	comedi_polynomial_t *from_phys;
} comedi_softcal_t;
#define CS_MAX_AREFS_LENGTH 4
typedef struct
{
	unsigned int subdevice;
	unsigned int *channels;
	unsigned int num_channels;
	unsigned int *ranges;
	unsigned int num_ranges;
	unsigned int arefs[ CS_MAX_AREFS_LENGTH ];
	unsigned int num_arefs;
	comedi_caldac_t *caldacs;
	unsigned int num_caldacs;
	comedi_softcal_t soft_calibration;
} comedi_calibration_setting_t;

typedef struct
{
	char *driver_name;
	char *board_name;
	comedi_calibration_setting_t *settings;
	unsigned int num_settings;
} comedi_calibration_t;

comedi_calibration_t* comedi_parse_calibration_file( const char *cal_file_path );
int comedi_apply_parsed_calibration( comedi_t *dev, unsigned int subdev, unsigned int channel,
	unsigned int range, unsigned int aref, const comedi_calibration_t *calibration );
char* comedi_get_default_calibration_path( comedi_t *dev );
void comedi_cleanup_calibration( comedi_calibration_t *calibration );
int comedi_apply_calibration( comedi_t *dev, unsigned int subdev, unsigned int channel,
	unsigned int range, unsigned int aref, const char *cal_file_path);

/* New stuff to provide conversion between integers and physical values that
* can support software calibrations. */
enum comedi_conversion_direction
{
	COMEDI_TO_PHYSICAL,
	COMEDI_FROM_PHYSICAL
};
int comedi_get_softcal_converter(
	unsigned subdevice, unsigned channel, unsigned range,
	enum comedi_conversion_direction direction,
	const comedi_calibration_t *calibration, comedi_polynomial_t* SWIG_OUTPUT(polynomial));
int comedi_get_hardcal_converter(
	comedi_t *dev, unsigned subdevice, unsigned channel, unsigned range,
	enum comedi_conversion_direction direction, comedi_polynomial_t* SWIG_OUTPUT(polynomial));
double comedi_to_physical(lsampl_t data,
	const comedi_polynomial_t *conversion_polynomial);
lsampl_t comedi_from_physical(double data,
	const comedi_polynomial_t *conversion_polynomial);

int comedi_internal_trigger(comedi_t *dev, unsigned subd, unsigned trignum);
/* INSN_CONFIG wrappers */
int comedi_arm(comedi_t *device, unsigned subdevice, unsigned source);
int comedi_reset(comedi_t *device, unsigned subdevice);
int comedi_get_clock_source(comedi_t *device, unsigned subdevice, unsigned *SWIG_OUTPUT(clock), unsigned *SWIG_OUTPUT(period_ns));
int comedi_get_gate_source(comedi_t *device, unsigned subdevice, unsigned channel,
	unsigned gate, unsigned *SWIG_OUTPUT(source));
int comedi_get_routing(comedi_t *device, unsigned subdevice, unsigned channel, unsigned *SWIG_OUTPUT(routing));
int comedi_set_counter_mode(comedi_t *device, unsigned subdevice, unsigned channel, unsigned mode_bits);
int comedi_set_clock_source(comedi_t *device, unsigned subdevice, unsigned clock, unsigned period_ns);
int comedi_set_filter(comedi_t *device, unsigned subdevice, unsigned channel, unsigned filter);
int comedi_set_gate_source(comedi_t *device, unsigned subdevice, unsigned channel, unsigned gate_index, unsigned gate_source);
int comedi_set_other_source(comedi_t *device, unsigned subdevice, unsigned channel,
	unsigned other, unsigned source);
int comedi_set_routing(comedi_t *device, unsigned subdevice, unsigned channel, unsigned routing);
int comedi_get_hardware_buffer_size(comedi_t *device, unsigned subdevice, enum comedi_io_direction direction);

#endif

#ifdef __cplusplus
}
#endif

#endif

