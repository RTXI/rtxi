/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License as        *
 *   published by                                                          *
 *   the Free Software Foundation; either version 2.1 of the License, or   *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef __CALIB_H_
#define __CALIB_H_

#include "comedilib.h"
#include "../libcomedi_calibrate/comedi_calibrate_shared.h"
#if 0
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <getopt.h>
#include <ctype.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#endif

#define DPRINT(level,fmt,args...) do{if(verbose>=level)printf(fmt, ## args);}while(0)

#define N_CALDACS 64
#define N_OBSERVABLES 128
#define PREOBSERVE_DATA_LEN 10

static const int caldac_settle_usec = 100000;

typedef struct{
	char *name;

	comedi_insn preobserve_insn;
	lsampl_t preobserve_data[ PREOBSERVE_DATA_LEN ];

	comedi_insn observe_insn;

	int reference_source;
	double target;
}observable;

typedef struct calibration_setup_struct calibration_setup_t;
struct calibration_setup_struct {
	comedi_t *dev;
	int ad_subdev;
	int da_subdev;
	int eeprom_subdev;
	int caldac_subdev;
	int status;
	unsigned int sv_settling_time_ns;
	unsigned int sv_order;
	observable observables[ N_OBSERVABLES ];
	unsigned int n_observables;
	comedi_caldac_t caldacs[ N_CALDACS ];
	unsigned int n_caldacs;
	int (*do_cal) ( calibration_setup_t *setup );
	const char *cal_save_file_path;
	unsigned do_output : 1;
	comedi_calibration_t *old_calibration;
	comedi_calibration_t *new_calibration;
	void *private_data;
};

extern int verbose;

enum {
	STATUS_UNKNOWN = 0,
	STATUS_GUESS,
	STATUS_SOME,
	STATUS_DONE
};

/* high level */

void observe( calibration_setup_t *setup );
int preobserve( calibration_setup_t *setup, int obs);
void observable_dependence( calibration_setup_t *setup, int obs);
void measure_observable( calibration_setup_t *setup, int obs);
void reset_caldac( calibration_setup_t *setup, int caldac_index );
void reset_caldacs( calibration_setup_t *setup);

/* drivers */

extern char ni_id[];
extern char cb_id[];
extern char cb64_id[];
extern char ni_labpc_id[];

int ni_setup( calibration_setup_t*, const char *device_name );
int cb_setup( calibration_setup_t*, const char *device_name );
int cb64_setup( calibration_setup_t*, const char *device_name );
int ni_labpc_setup( calibration_setup_t*, const char *device_name );

/* low level */

void set_target( calibration_setup_t *setup, int obs,double target);
void update_caldac( calibration_setup_t *setup, int caldac_index, int value );
void setup_caldacs( calibration_setup_t *setup, int caldac_subdev);
void postgain_cal( calibration_setup_t *setup, int obs1, int obs2, int dac);
void cal1( calibration_setup_t *setup, int obs, int dac);
void cal1_fine( calibration_setup_t *setup, int obs, int dac);
void cal_binary( calibration_setup_t *setup, int obs, int dac);
void cal_postgain_binary( calibration_setup_t *setup, int obs1, int obs2, int dac);
void cal_relative_binary( calibration_setup_t *setup, int obs1, int obs2, int dac);
void cal_linearity_binary( calibration_setup_t *setup, int obs1, int obs2, int obs3, int dac);
void peg_binary( calibration_setup_t *setup, int obs, int dac, int maximize );

/* misc and temp */

void channel_dependence(int adc,int range);
void caldac_dependence(int caldac);
void chan_cal(int adc,int caldac,int range,double target);
int read_eeprom( calibration_setup_t *setup, int addr);

double read_chan( calibration_setup_t *setup, int adc,int range);
int read_chan2( calibration_setup_t *setup, char *s,int adc,int range);
void set_ao(comedi_t *dev,int subdev,int chan,int range,double value);
void check_gain(int ad_chan,int range);
double check_gain_chan(int ad_chan,int range,int cdac);

int cb_actual_source_voltage( comedi_t *dev, unsigned int subdevice,
	unsigned int eeprom_channel, float *voltage);

/* helper functions */

int get_bipolar_lowgain(comedi_t *dev,int subdev);
int get_bipolar_highgain(comedi_t *dev,int subdev);
int get_unipolar_lowgain(comedi_t *dev,int subdev);
int get_unipolar_highgain(comedi_t *dev,int subdev);
double very_low_target( comedi_t *dev, unsigned int subdevice,
	unsigned int channel, unsigned int range );
int is_bipolar( comedi_t *dev, unsigned int subdevice,
	unsigned int channel, unsigned int range );
int is_unipolar( comedi_t *dev, unsigned int subdevice,
	unsigned int channel, unsigned int range );

double fractional_offset( calibration_setup_t *setup, int subdevice,
	unsigned int channel, unsigned int range, int obs );
double get_tolerance( calibration_setup_t *setup, int subdevice,
	double num_bits );

/* other */

void comedi_nanodelay(comedi_t *dev, unsigned int delay);
unsigned caldac_maxdata(comedi_t *dev, const comedi_caldac_t *caldac);

/* printing scientific numbers */

int sci_sprint(char *s,double x,double y);
int sci_sprint_alt(char *s,double x,double y);

/* linear fitting */

typedef struct {
	int n;

	double *y_data;
	double *yerr_data;
	double *x_data;

	double x0;
	double dx;
	double yerr;

	/* stats */
	double s1,sx,sy,sxy,sxx;

	double min,max;

	/* results */
	double ave_x;
	double ave_y;
	double slope;
	double err_slope;
	double err_ave_y;
	double S_min;
	double dof;

}linear_fit_t;
int linear_fit_monotonic(linear_fit_t *l);
double linear_fit_func_y(linear_fit_t *l,double x);
double linear_fit_func_x(linear_fit_t *l,double y);
double check_gain_chan_x( calibration_setup_t *setup, linear_fit_t *l,unsigned int ad_chanspec,int cdac);
double check_gain_chan_fine( calibration_setup_t *setup, linear_fit_t *l,unsigned int ad_chanspec,int cdac);
void dump_curve(linear_fit_t *l);

/* slowly varying measurements */

typedef struct{
	comedi_t *dev;

	int maxdata;
	int order;
	int subd;
	unsigned int chanspec;
	unsigned int settling_time_ns;

	comedi_range *rng;

	double average;
	double stddev;
	double error;
}new_sv_t;

int new_sv_measure(comedi_t *dev, new_sv_t *sv);
int new_sv_init(new_sv_t *sv,comedi_t *dev,int subdev,unsigned int chanspec);
int my_sv_init( new_sv_t *sv, const calibration_setup_t *setup, int subdev,
	unsigned int chanspec );

/* generic calibration support */
typedef struct
{
	int (*adc_offset)( unsigned int channel );
	int (*adc_offset_fine)( unsigned int channel );
	int (*adc_postgain_offset)( unsigned int channel );
	int (*adc_gain)( unsigned int channel );
	int (*adc_gain_fine)( unsigned int channel );
	int (*dac_linearity)( unsigned int channel );
	int (*dac_linearity_fine)( unsigned int channel );
	int (*dac_offset)( unsigned int channel );
	int (*dac_offset_fine)( unsigned int channel );
	int (*dac_gain)( unsigned int channel );
	int (*dac_gain_fine)( unsigned int channel );
	int (*adc_high_observable)( const calibration_setup_t *setup,
		unsigned int channel, unsigned int range );
	int (*adc_ground_observable)( const calibration_setup_t *setup,
		unsigned int channel, unsigned int range );
	int (*dac_high_observable)( const calibration_setup_t *setup,
		unsigned int channel, unsigned int range );
	int (*dac_mid_observable)( const calibration_setup_t *setup,
		unsigned int channel, unsigned int range );
	int (*dac_ground_observable)( const calibration_setup_t *setup,
		unsigned int channel, unsigned int range );
	double adc_fractional_tolerance;
	double dac_fractional_tolerance;
	unsigned do_adc_unipolar_postgain : 1;
} generic_layout_t;
void init_generic_layout( generic_layout_t *layout );
int generic_cal_by_channel_and_range( calibration_setup_t *setup,
	const generic_layout_t *layout  );
int generic_cal_by_range( calibration_setup_t *setup,
	const generic_layout_t *layout  );
int generic_cal_ao(calibration_setup_t *setup,
	const generic_layout_t *layout  );
void generic_do_cal( calibration_setup_t *setup,
	comedi_calibration_setting_t *saved_cal, int observable, int caldac );
void generic_do_relative( calibration_setup_t *setup,
	comedi_calibration_setting_t *saved_cal, int observable1, int observable2, int caldac );
void generic_do_linearity( calibration_setup_t *setup,
	comedi_calibration_setting_t *saved_cal, int observable1, int observable2,
	int observable3, int caldac );
void generic_prep_adc_caldacs( calibration_setup_t *setup,
	const generic_layout_t *layout, unsigned int channel, unsigned int range );
void generic_prep_dac_caldacs( calibration_setup_t *setup,
	const generic_layout_t *layout, unsigned int channel, unsigned int range );
void generic_peg( calibration_setup_t *setup, int observable, int caldac,
	int maximize );

#endif

