/*
   A little auto-calibration utility, for boards
   that support it.

   copyright (C) 1999,2000,2001,2002 by David Schleef
   copyright (C) 2003 by Frank Mori Hess

 */

 /***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License as        *
 *   published by                                                          *
 *   the Free Software Foundation; either version 2.1 of the License, or   *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#define _GNU_SOURCE

#include <stdio.h>
#include "comedilib.h"
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <getopt.h>
#include <ctype.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "calib.h"

/* global variables */
int verbose = 0;

struct board_struct{
	char *name;
	char *id;
	int (*init_setup)( calibration_setup_t *setup, const char *device_name );
};

struct board_struct drivers[] = {
	{ "ni_pcimio",	ni_id,	ni_setup },
	{ "ni_atmio",	ni_id,	ni_setup },
	{ "ni_mio_cs",	ni_id,	ni_setup },
	{ "cb_pcidas",	cb_id,	cb_setup },
	{ "cb_pcidas64",	cb64_id,	cb64_setup },
	{ "ni_labpc", ni_labpc_id,	ni_labpc_setup },
};
#define n_drivers (sizeof(drivers)/sizeof(drivers[0]))

void help(void)
{
	printf("comedi_calibrate [options] - autocalibrates a Comedi device\n");
	printf("  --verbose, -v  \n");
	printf("  --quiet, -q  \n");
	printf("  --help, -h  \n");
	printf("  --file, -f [/dev/comediN] \n");
	printf("  --save-file, -S [filepath] \n");
	printf("  --driver-name [driver]  \n");
	printf("  --device-name [device]  \n");
	printf("  --[no-]reset  \n");
	printf("  --[no-]calibrate  \n");
	printf("  --[no-]dump  \n");
	printf("  --[no-]results  \n");
	printf("  --[no-]output  \n");
}

typedef struct
{
	int verbose;
	const char *file_path;
	const char *save_file_path;
	const char *driver_name;
	const char *device_name;
	int do_reset;
	int do_dump;
	int do_calibrate;
	int do_results;
	int do_output;
	unsigned int subdevice;
	unsigned int channel;
	unsigned int range;
	unsigned int aref;
} parsed_options_t;

static void parse_options( int argc, char *argv[], parsed_options_t *settings )
{
	int c, index;

	struct option options[] = {
		{ "verbose", 0, 0, 'v' },
		{ "quiet", 0, 0, 'q' },
		{ "file", 1, 0, 'f' },
		{ "save-file", 1, 0, 'S' },
		{ "help", 0, 0, 'h' },
		{ "driver-name", 1, 0, 0x1000 },
		{ "device-name", 1, 0, 0x1001 },
		{ "reset", 0, &settings->do_reset, 1 },
		{ "no-reset", 0, &settings->do_reset, 0 },
		{ "calibrate", 0, &settings->do_calibrate, 1 },
		{ "no-calibrate", 0, &settings->do_calibrate, 0 },
		{ "dump", 0, &settings->do_dump, 1 },
		{ "no-dump", 0, &settings->do_dump, 0 },
		{ "results", 0, &settings->do_results, 1 },
		{ "no-results", 0, &settings->do_results, 0 },
		{ "output", 0, &settings->do_output, 1 },
		{ "no-output", 0, &settings->do_output, 0 },
		{ "subdevice", 1, 0, 's' },
		{ "channel", 1, 0, 'c' },
		{ "range", 1, 0, 'r' },
		{ "aref", 1, 0, 'a' },
		{ 0 },
	};

	while (1) {
		c = getopt_long(argc, argv, "f:S:vqs:c:r:a:", options, &index);
		if (c == -1)break;
		switch (c) {
		case 0:
			continue;
		case 'h':
			help();
			exit(0);
			break;
		case 'f':
			settings->file_path = optarg;
			break;
		case 'S':
			settings->save_file_path = optarg;
			break;
		case 'v':
			settings->verbose++;
			break;
		case 'q':
			settings->verbose--;
			break;
		case 0x1000:
			settings->driver_name = optarg;
			break;
		case 0x1001:
			settings->device_name = optarg;
			break;
		case 's':
			settings->subdevice = strtoul( optarg, NULL, 0 );
			break;
		case 'c':
			settings->channel = strtoul( optarg, NULL, 0 );
			break;
		case 'r':
			settings->range = strtoul( optarg, NULL, 0 );
			break;
		case 'a':
			settings->aref = strtoul( optarg, NULL, 0 );
			break;
		default:
			help();
			exit(1);
		}
	}
}

int main(int argc, char *argv[])
{
	int i;
	struct board_struct *this_board;
	int device_status = STATUS_UNKNOWN;
	calibration_setup_t setup;
	int retval;
	parsed_options_t options;

	memset( &setup, 0, sizeof( setup ) );
	setup.sv_settling_time_ns = 99999;
	setup.sv_order = 10;

	memset( &options, 0, sizeof( options ) );
	options.do_dump = 0;
	options.do_reset = 0;
	options.do_calibrate = -1;
	options.do_results = 0;
	options.do_output = 1;
	options.file_path = "/dev/comedi0";
	parse_options( argc, argv, &options );
	verbose = options.verbose;

	setup.dev = comedi_open( options.file_path );
	if( setup.dev == NULL ) {
		fprintf( stderr, "comedi_open() failed, with device file name: %s\n",
			options.file_path );
		comedi_perror("comedi_open");
		exit(0);
	}

	if( options.save_file_path == NULL )
		options.save_file_path = comedi_get_default_calibration_path( setup.dev );
	if(!options.driver_name)
		options.driver_name = comedi_get_driver_name( setup.dev );
	if(!options.device_name)
		options.device_name = comedi_get_board_name( setup.dev );

	setup.ad_subdev=comedi_find_subdevice_by_type( setup.dev,COMEDI_SUBD_AI,0);
	setup.da_subdev=comedi_find_subdevice_by_type( setup.dev,COMEDI_SUBD_AO,0);
	setup.caldac_subdev=comedi_find_subdevice_by_type( setup.dev,COMEDI_SUBD_CALIB,0);
	setup.eeprom_subdev=comedi_find_subdevice_by_type( setup.dev,COMEDI_SUBD_MEMORY,0);

	for(i=0;i<n_drivers;i++){
		if(!strcmp(drivers[i].name,options.driver_name)){
			this_board = drivers+i;
			goto ok;
		}
	}
	fprintf(stderr, "Driver %s unknown\n", options.driver_name);
	return 1;

ok:

	retval = this_board->init_setup( &setup, options.device_name );
	if( retval < 0 ){
		fprintf(stderr, "init_setup() failed for %s\n", options.device_name );
		return 1;
	}
	device_status = setup.status;

	if(device_status<STATUS_DONE){
		printf("Warning: device may not be not fully calibrated due to "
			"insufficient information.\n"
			"Please file a bug report at https://bugs.comedi.org/ and attach this output.\n"
			"This output will also allow comedi_calibrate to execute more\n"
			"quickly in the future.\n");
		if(verbose<1){
			verbose=1;
			printf("Forcing option: --verbose\n");
		}
		if(device_status==STATUS_UNKNOWN){
			options.do_reset=1;
			options.do_dump=1;
			options.do_calibrate=0;
			options.do_results=0;
			printf("Forcing options: --reset --dump --no-calibrate --no-results\n");
		}
		if(device_status==STATUS_SOME){
			options.do_reset=1;
			options.do_dump=1;
			options.do_calibrate=1;
			options.do_results=1;
			printf("Forcing options: --reset --dump --calibrate --results\n");
		}
		if(device_status==STATUS_GUESS){
			options.do_reset=1;
			options.do_dump=1;
			options.do_calibrate=1;
			options.do_results=1;
			printf("Forcing options: --reset --dump --calibrate --results\n");
		}
	}
	if(verbose>=0){
		char *s = "$Id: comedi_calibrate.c,v 1.8 2008-01-11 13:29:19 abbotti Exp $";

		printf("%.*s\n",(int)strlen(s)-2,s+1);
		printf("Driver name: %s\n", options.driver_name);
		printf("Device name: %s\n", options.device_name);
		printf("%.*s\n",(int)strlen(this_board->id)-2,this_board->id+1);
		printf("Comedi version: %d.%d.%d\n",
			(comedi_get_version_code(setup.dev)>>16)&0xff,
			(comedi_get_version_code(setup.dev)>>8)&0xff,
			(comedi_get_version_code(setup.dev))&0xff);
	}

	if( options.do_reset == 0 )
		setup.old_calibration = comedi_parse_calibration_file( options.save_file_path );
	else
		setup.old_calibration = NULL;
	if( options.do_calibrate < 0 )
	{
		if( setup.old_calibration ) options.do_calibrate = 0;
		else options.do_calibrate = 1;
	}
	setup.do_output = options.do_output;
	setup.cal_save_file_path = options.save_file_path;

	if(options.do_dump) observe( &setup );
	if(options.do_calibrate && setup.do_cal)
	{
		setup.new_calibration = malloc( sizeof( comedi_calibration_t ) );
		assert( setup.new_calibration );
		memset( setup.new_calibration, 0, sizeof( comedi_calibration_t ) );
		setup.new_calibration->driver_name = strdup( comedi_get_driver_name( setup.dev ) );
		assert( setup.new_calibration->driver_name != NULL );
		setup.new_calibration->board_name = strdup( comedi_get_board_name( setup.dev ) );
		assert( setup.new_calibration->board_name != NULL );
		retval = setup.do_cal( &setup );
		if( retval < 0 )
		{
			fprintf( stderr, "calibration function returned error\n" );
			return -1;
		}
	}
	if(options.do_results) observe( &setup );

	if( setup.old_calibration ) comedi_cleanup_calibration( setup.old_calibration );
	if( setup.new_calibration ) comedi_cleanup_calibration( setup.new_calibration );

	retval = comedi_apply_calibration( setup.dev, options.subdevice,
		options.channel, options.range, options.aref, setup.cal_save_file_path );
	if( retval < 0 )
	{
		DPRINT( 0, "Failed to apply " );
	}else
	{
		DPRINT( 0, "Applied " );
	}
	DPRINT( 0, "calibration for subdevice %i, channel %i, range %i, aref %i\n",
		options.subdevice, options.channel, options.range, options.aref );
	comedi_close(setup.dev);

	return retval;
}

void set_target( calibration_setup_t *setup, int obs, double target)
{
	comedi_range *range;
	lsampl_t maxdata, data;

	comedi_set_global_oor_behavior( COMEDI_OOR_NUMBER );

	range = comedi_get_range(setup->dev,
		setup->observables[obs].preobserve_insn.subdev,
		CR_CHAN( setup->observables[obs].preobserve_insn.chanspec ),
		CR_RANGE( setup->observables[obs].preobserve_insn.chanspec ));
	assert( range );
	maxdata = comedi_get_maxdata( setup->dev,
		setup->observables[obs].preobserve_insn.subdev,
		CR_CHAN(setup->observables[obs].preobserve_insn.chanspec));
	assert( maxdata > 0 );
	data = comedi_from_phys(target,range,maxdata);

	setup->observables[obs].preobserve_data[0] = data;
	setup->observables[obs].target = comedi_to_phys(data,range,maxdata);
}

static void apply_appropriate_cal( calibration_setup_t *setup, comedi_insn insn )
{
	int retval = 0;

	if( setup->new_calibration )
	{
		retval = comedi_apply_parsed_calibration( setup->dev, insn.subdev,
			CR_CHAN( insn.chanspec ), CR_RANGE( insn.chanspec ),
			CR_AREF( insn.chanspec ), setup->new_calibration );
	}else if( setup->old_calibration )
	{
		retval = comedi_apply_parsed_calibration( setup->dev, insn.subdev,
			CR_CHAN( insn.chanspec ), CR_RANGE( insn.chanspec ),
			CR_AREF( insn.chanspec ), setup->old_calibration );
	}else
	{
		reset_caldacs( setup );
		return;
	}
	if( retval < 0 )
		DPRINT( 1, "Failed to apply ");
	else
		DPRINT( 1, "Applied ");
	DPRINT( 1, "calibration for subdev %i, channel %i, range %i, aref %i\n", insn.subdev,
		CR_CHAN( insn.chanspec ), CR_RANGE( insn.chanspec ),
		CR_AREF( insn.chanspec ) );
}

void observe( calibration_setup_t *setup )
{
	int i;
	observable *obs;

	for( i = 0; i < setup->n_observables; i++){
		obs = &setup->observables[ i ];
		if( obs->observe_insn.n == 0 ) continue;
		preobserve( setup, i);
		DPRINT(0,"%s\n", setup->observables[i].name);
		if( obs->preobserve_insn.n != 0){
			apply_appropriate_cal( setup, obs->preobserve_insn );
		}
		apply_appropriate_cal( setup, obs->observe_insn );
		measure_observable( setup, i);
		if(verbose>=1){
			observable_dependence( setup, i);
		}
	}
}

int preobserve( calibration_setup_t *setup, int obs)
{
	int retval;
	comedi_insn reference_source_config;
	lsampl_t ref_data[ 2 ];

	// setup reference source
	if(setup->observables[obs].reference_source >= 0)
	{
		memset( &reference_source_config, 0, sizeof(reference_source_config) );
		reference_source_config.insn = INSN_CONFIG;
		reference_source_config.n = 2;
		reference_source_config.subdev = setup->ad_subdev;
		reference_source_config.data = ref_data;
		reference_source_config.data[ 0 ] = INSN_CONFIG_ALT_SOURCE;
		reference_source_config.data[ 1 ] = setup->observables[obs].reference_source;

		retval = comedi_do_insn( setup->dev, &reference_source_config );
		/* ignore errors for now since older ni driver doesn't
		* support reference config insn */
		if( retval < 0 )
			perror("preobserve() ignoring reference config error" );
	}
	retval = 0;

	if( setup->observables[obs].preobserve_insn.n != 0){
		retval = comedi_do_insn( setup->dev, &setup->observables[obs].preobserve_insn);
	}
	if( retval < 0 )
		perror("preobserve()");

	return retval;
}

void measure_observable( calibration_setup_t *setup, int obs)
{
	char s[100];
	int n;
	new_sv_t sv;

	my_sv_init(&sv, setup,
		setup->observables[obs].observe_insn.subdev,
		setup->observables[obs].observe_insn.chanspec);
	n = new_sv_measure(setup->dev, &sv);

	sci_sprint_alt(s,sv.average,sv.error);
	DPRINT(0,"reading %s, target %g\n",s, setup->observables[obs].target);
	assert( isnan( setup->observables[obs].target) == 0 );
}

void observable_dependence(calibration_setup_t *setup, int obs)
{
	int i;
	linear_fit_t l;

	for( i = 0; i < setup->n_caldacs; i++){
		check_gain_chan_x( setup, &l,
			setup->observables[obs].observe_insn.chanspec, i);
	}

}


void postgain_cal( calibration_setup_t *setup, int obs1, int obs2, int dac)
{
	double offset1,offset2;
	linear_fit_t l;
	double slope1,slope2;
	double a;
	double gain;
	comedi_range *range1,*range2;

	DPRINT(0,"postgain: %s; %s\n", setup->observables[obs1].name,
		setup->observables[obs2].name);
	preobserve(setup, obs1);
	check_gain_chan_x( setup, &l, setup->observables[obs1].observe_insn.chanspec, dac);
	offset1=linear_fit_func_y(&l, setup->caldacs[dac].value);
	DPRINT(2,"obs1: [%d] offset %g\n",obs1,offset1);
	range1 = comedi_get_range(setup->dev, setup->observables[obs1].observe_insn.subdev,
		CR_CHAN( setup->observables[obs1].observe_insn.chanspec),
		CR_RANGE( setup->observables[obs1].observe_insn.chanspec));
	slope1=l.slope;

	preobserve( setup, obs2);
	check_gain_chan_x( setup, &l, setup->observables[obs2].observe_insn.chanspec,dac);
	offset2=linear_fit_func_y(&l, setup->caldacs[dac].value);
	DPRINT(2,"obs2: [%d] offset %g\n",obs2,offset2);
	range2 = comedi_get_range(setup->dev, setup->observables[obs2].observe_insn.subdev,
		CR_CHAN( setup->observables[obs2].observe_insn.chanspec),
		CR_RANGE( setup->observables[obs2].observe_insn.chanspec));
	slope2=l.slope;

	gain = (range1->max-range1->min)/(range2->max-range2->min);
	DPRINT(4,"range1 %g range2 %g\n", range1->max-range1->min,
		range2->max-range2->min);
	DPRINT(3,"gain: %g\n",gain);

	DPRINT(3,"difference: %g\n",offset2-offset1);

	a = (offset1-offset2)/(slope1-slope2);
	a = setup->caldacs[dac].value - a;

	update_caldac( setup, dac, rint(a) );
	usleep(caldac_settle_usec);

	DPRINT(0,"caldac[%d] set to %g (%g)\n",dac,rint(a),a);

	if(verbose>=2){
		preobserve( setup, obs1);
		measure_observable( setup, obs1);
		preobserve( setup, obs2);
		measure_observable( setup, obs2);
	}
}

void cal1( calibration_setup_t *setup, int obs, int dac)
{
	linear_fit_t l;
	double a;

	DPRINT(0,"linear: %s\n", setup->observables[obs].name);
	preobserve( setup, obs);
	check_gain_chan_x( setup, &l, setup->observables[obs].observe_insn.chanspec,dac);
	a=linear_fit_func_x(&l, setup->observables[obs].target);

	update_caldac( setup, dac, rint(a) );
	usleep(caldac_settle_usec);

	DPRINT(0,"caldac[%d] set to %g (%g)\n",dac,rint(a),a);
	if(verbose>=3){
		measure_observable( setup, obs);
	}
}

void cal1_fine( calibration_setup_t *setup, int obs, int dac )
{
	linear_fit_t l;
	double a;

	DPRINT(0,"linear fine: %s\n", setup->observables[obs].name);
	preobserve( setup, obs);
	check_gain_chan_fine( setup, &l, setup->observables[obs].observe_insn.chanspec,dac);
	a=linear_fit_func_x(&l,setup->observables[obs].target);

	update_caldac( setup, dac, rint(a) );
	usleep(caldac_settle_usec);

	DPRINT(0,"caldac[%d] set to %g (%g)\n",dac,rint(a),a);
	if(verbose>=3){
		measure_observable( setup, obs);
	}
}

void peg_binary( calibration_setup_t *setup, int obs, int dac, int maximize )
{
	int x0, x1, x;
	double y0, y1;
	new_sv_t sv;
	unsigned int chanspec = setup->observables[obs].observe_insn.chanspec;
	int polarity;

	DPRINT(0,"binary peg: %s\n", setup->observables[obs].name);
	preobserve( setup, obs);

	comedi_set_global_oor_behavior( COMEDI_OOR_NUMBER );

	my_sv_init(&sv, setup, setup->ad_subdev, chanspec);

	x0 = caldac_maxdata(setup->dev, &setup->caldacs[dac]);
	update_caldac( setup, dac, x0 );
	usleep(caldac_settle_usec);
	new_sv_measure( setup->dev, &sv);
	y0 = sv.average;

	x1 = 0;
	update_caldac( setup, dac, x1 );
	usleep(caldac_settle_usec);
	new_sv_measure( setup->dev, &sv);
	y1 = sv.average;

	if( (y0 - y1) > 0.0 ) polarity = 1;
	else polarity = -1;

	if( maximize )
	{
		if( polarity > 0 ) x = x0;
		else x = x1;
	}else
	{
		if( polarity > 0 ) x = x1;
		else x = x0;
	}
	update_caldac( setup, dac, x );
	DPRINT(0,"caldac[%d] set to %d\n",dac,x);
	if(verbose>=3){
		measure_observable( setup, obs);
	}
}

void cal_binary( calibration_setup_t *setup, int obs, int dac)
{
	int x0, x1, x2, x;
	unsigned int bit;
	double y0, y1, y2;
	new_sv_t sv;
	double target = setup->observables[obs].target;
	unsigned int chanspec = setup->observables[obs].observe_insn.chanspec;
	int polarity;

	DPRINT(0,"binary: %s\n", setup->observables[obs].name);
	preobserve( setup, obs);

	comedi_set_global_oor_behavior( COMEDI_OOR_NUMBER );

	my_sv_init(&sv, setup, setup->ad_subdev, chanspec);

	x0 = caldac_maxdata(setup->dev, &setup->caldacs[dac]);
	update_caldac( setup, dac, x0 );
	usleep(caldac_settle_usec);
	new_sv_measure( setup->dev, &sv);
	y0 = sv.average;

	x1 = x2 = 0;
	update_caldac( setup, dac, x1 );
	usleep(caldac_settle_usec);
	new_sv_measure( setup->dev, &sv);
	y1 = y2 = sv.average;

	if( (y0 - y1) > 0.0 ) polarity = 1;
	else polarity = -1;

	bit = 1;
	while((bit << 1) < caldac_maxdata(setup->dev, &setup->caldacs[dac]))
		bit <<= 1;
	for( ; bit; bit >>= 1 ){
		x2 = x1 | bit;

		update_caldac( setup, dac, x2 );
		usleep(caldac_settle_usec);
		new_sv_measure( setup->dev, &sv);
		y2 = sv.average;
		DPRINT(3,"trying %d, result %g, target %g\n",x2,y2,target);

		if( (y2 - target) * polarity < 0.0 ){
			x1 = x2;
			y1 = y2;
		}

		if(verbose>=3){
			measure_observable( setup, obs);
		}
	}

	// get that least signficant bit right
	if( fabs( y1 - target ) < fabs( y2 - target ) )
		x = x1;
	else x = x2;

	update_caldac( setup, dac, x );
	DPRINT(0,"caldac[%d] set to %d\n",dac,x);
	if( x >= caldac_maxdata(setup->dev, &setup->caldacs[dac]) || x <= 0 )
		DPRINT(0,"WARNING: caldac[%d] pegged!\n", dac );
	if(verbose>=3){
		measure_observable( setup, obs);
	}
}

void cal_postgain_binary( calibration_setup_t *setup, int obs1, int obs2, int dac)
{
	cal_relative_binary( setup, obs1, obs2, dac );
}

void cal_relative_binary( calibration_setup_t *setup, int obs1, int obs2, int dac)
{
	int x0, x1, x2, x, polarity;
	double y0, y1, y2;
	new_sv_t sv1, sv2;
	double target = setup->observables[obs1].target - setup->observables[obs2].target;
	unsigned int chanspec1 = setup->observables[obs1].observe_insn.chanspec;
	unsigned int chanspec2 = setup->observables[obs2].observe_insn.chanspec;
	unsigned int bit;

	DPRINT(0,"relative binary: %s, %s\n", setup->observables[obs1].name,
		setup->observables[obs2].name);

	comedi_set_global_oor_behavior( COMEDI_OOR_NUMBER );

	x0 = caldac_maxdata(setup->dev, &setup->caldacs[dac]);
	update_caldac( setup, dac, x0 );
	usleep(caldac_settle_usec);
	preobserve( setup, obs1);
	my_sv_init(&sv1, setup, setup->ad_subdev,chanspec1);
	new_sv_measure( setup->dev, &sv1);

	preobserve( setup, obs2);
	my_sv_init(&sv2, setup, setup->ad_subdev,chanspec2);
	new_sv_measure( setup->dev, &sv2);
	y0 = sv1.average - sv2.average;

	x1 = x2 = 0;
	update_caldac( setup, dac, x1 );
	usleep(caldac_settle_usec);
	preobserve( setup, obs1);
	new_sv_measure( setup->dev, &sv1);

	preobserve( setup, obs2);
	new_sv_measure( setup->dev, &sv2);
	y1 = y2 = sv1.average - sv2.average;

	if( (y0 - y1) > 0.0 ) polarity = 1;
	else polarity = -1;

	bit = 1;
	while((bit << 1) < caldac_maxdata(setup->dev, &setup->caldacs[dac]))
		bit <<= 1;
	for( ; bit; bit >>= 1 )
	{
		x2 = x1 | bit;

		update_caldac( setup, dac, x2 );
		usleep(caldac_settle_usec);

		preobserve( setup, obs1);
		new_sv_measure( setup->dev, &sv1);

		preobserve( setup, obs2);
		new_sv_measure( setup->dev, &sv2);
		y2 = sv1.average - sv2.average;

		DPRINT(3,"trying %d, result %g, target %g\n",x2,y2,target);

		if( (y2 - target) * polarity < 0.0 ){
			x1 = x2;
			y1 = y2;
		}

		if(verbose>=3){
			preobserve( setup, obs1);
			measure_observable( setup, obs1);
			preobserve( setup, obs2);
			measure_observable( setup, obs2);
		}
	}

	if( fabs( y1 - target ) < fabs( y2 - target ) )
		x = x1;
	else
		x = x2;
	update_caldac( setup, dac, x );
	DPRINT(0,"caldac[%d] set to %d\n",dac,x);
	if(x >= caldac_maxdata(setup->dev, &setup->caldacs[dac]) || x <= 0 )
		DPRINT(0,"WARNING: caldac[%d] pegged!\n", dac );
	if(verbose>=3){
		preobserve( setup, obs1);
		measure_observable( setup, obs1);
		preobserve( setup, obs2);
		measure_observable( setup, obs2);
	}
}

void cal_linearity_binary( calibration_setup_t *setup, int obs1, int obs2, int obs3, int dac)
{
	int x0, x1, x2, x, polarity;
	double y0, y1, y2;
	new_sv_t sv1, sv2, sv3;
	double target = ( setup->observables[obs3].target - setup->observables[obs2].target ) /
		( setup->observables[obs2].target - setup->observables[obs1].target );
	unsigned int chanspec1 = setup->observables[obs1].observe_insn.chanspec;
	unsigned int chanspec2 = setup->observables[obs2].observe_insn.chanspec;
	unsigned int chanspec3 = setup->observables[obs3].observe_insn.chanspec;
	unsigned int bit;

	DPRINT(0,"linearity binary: %s,\n%s,\n%s\n", setup->observables[obs1].name,
		setup->observables[obs2].name,setup->observables[obs3].name);

	comedi_set_global_oor_behavior( COMEDI_OOR_NUMBER );

	x0 = caldac_maxdata(setup->dev, &setup->caldacs[dac]);
	update_caldac( setup, dac, x0 );
	usleep(caldac_settle_usec);

	preobserve( setup, obs1);
	my_sv_init(&sv1, setup, setup->ad_subdev,chanspec1);
	new_sv_measure( setup->dev, &sv1);

	preobserve( setup, obs2);
	my_sv_init(&sv2, setup, setup->ad_subdev,chanspec2);
	new_sv_measure( setup->dev, &sv2);

	preobserve( setup, obs3);
	my_sv_init(&sv3, setup, setup->ad_subdev,chanspec3);
	new_sv_measure( setup->dev, &sv3);

	y0 = ( sv3.average - sv2.average ) / ( sv2.average - sv1.average );

	x1 = x2 = 0;
	update_caldac( setup, dac, x1 );
	usleep(caldac_settle_usec);

	preobserve( setup, obs1);
	new_sv_measure( setup->dev, &sv1);

	preobserve( setup, obs2);
	new_sv_measure( setup->dev, &sv2);

	preobserve( setup, obs3);
	new_sv_measure( setup->dev, &sv3);

	y1 = y2 = ( sv3.average - sv2.average ) / ( sv2.average - sv1.average );

	if( (y0 - y1) > 0.0 ) polarity = 1;
	else polarity = -1;

	bit = 1;
	while((bit << 1) < caldac_maxdata(setup->dev, &setup->caldacs[dac]))
		bit <<= 1;
	for( ; bit; bit >>= 1 )
	{
		x2 = x1 | bit;

		update_caldac( setup, dac, x2 );
		usleep(caldac_settle_usec);

		preobserve( setup, obs1);
		new_sv_measure( setup->dev, &sv1);

		preobserve( setup, obs2);
		new_sv_measure( setup->dev, &sv2);

		preobserve( setup, obs3);
		new_sv_measure( setup->dev, &sv3);

		y2 = ( sv3.average - sv2.average ) / ( sv2.average - sv1.average );

		DPRINT(3,"trying %d, result %g, target %g\n",x2,y2,target);

		if( (y2 - target) * polarity < 0.0 ){
			x1 = x2;
			y1 = y2;
		}

		if(verbose>=3){
			preobserve( setup, obs1);
			measure_observable( setup, obs1);
			preobserve( setup, obs2);
			measure_observable( setup, obs2);
			preobserve( setup, obs3);
			measure_observable( setup, obs3);
		}
	}

	if( fabs( y1 - target ) < fabs( y2 - target ) )
		x = x1;
	else
		x = x2;
	update_caldac( setup, dac, x );
	DPRINT(0,"caldac[%d] set to %d\n",dac,x);
	if(x >= caldac_maxdata(setup->dev, &setup->caldacs[dac]) || x <= 0 )
		DPRINT(0,"WARNING: caldac[%d] pegged!\n", dac );
	if(verbose>=3){
		preobserve( setup, obs1);
		measure_observable( setup, obs1);
		preobserve( setup, obs2);
		measure_observable( setup, obs2);
		preobserve( setup, obs3);
		measure_observable( setup, obs3);
	}
}

#if 0
void chan_cal(int adc,int cdac,int range,double target)
{
	linear_fit_t l;
	double offset;
	double gain;
	double a;
	char s[32];

	check_gain_chan_x(&l,CR_PACK(adc,range,AREF_OTHER),cdac);
	offset=linear_fit_func_y(&l,caldacs[cdac].current);
	gain=l.slope;

	a=caldacs[cdac].current+(target-offset)/gain;

	update_caldac( setup, cdac, rint(a));

	read_chan2(s,adc,range);
	DPRINT(1,"caldac[%d] set to %g, offset=%s\n",cdac,a,s);
}
#endif


#if 0
void channel_dependence(int adc,int range)
{
	int i;
	double gain;

	for(i=0;i<n_caldacs;i++){
		gain=check_gain_chan(adc,range,i);
	}
}
#endif

#if 0
void caldac_dependence(int caldac)
{
	int i;
	double gain;

	for(i=0;i<8;i++){
		gain=check_gain_chan(i,0,caldac);
	}
}
#endif


void setup_caldacs( calibration_setup_t *setup, int caldac_subdev )
{
	int n_chan, i;

	if( caldac_subdev < 0 ){
		printf("no calibration subdevice\n");
		return;
	}

	// XXX check subdevice type is really calibration

	n_chan = comedi_get_n_channels( setup->dev, caldac_subdev );
	assert(n_chan >= 0);
	assert(setup->n_caldacs + n_chan < N_CALDACS);

	for(i = 0; i < n_chan; i++){
		setup->caldacs[ setup->n_caldacs + i ].subdevice = caldac_subdev;
		setup->caldacs[ setup->n_caldacs + i ].channel = i;
		setup->caldacs[ setup->n_caldacs + i ].value = 0;
	}

	setup->n_caldacs += n_chan;
}

void reset_caldac( calibration_setup_t *setup, int caldac_index )
{
	if( caldac_index < 0 ) return;
	assert( caldac_index < setup->n_caldacs );
	update_caldac(setup, caldac_index, caldac_maxdata(setup->dev, &setup->caldacs[caldac_index]) / 2);
}

void reset_caldacs( calibration_setup_t *setup )
{
	int i;

	for( i = 0; i < setup->n_caldacs; i++){
		reset_caldac( setup, i );
	}
}

void update_caldac( calibration_setup_t *setup, int caldac_index,
	int value )
{
	int ret;
	comedi_caldac_t *dac;

	if( caldac_index < 0 ) return;
	if( caldac_index >= setup->n_caldacs )
	{
		fprintf( stderr, "invalid caldac index\n" );
		return;
	}
	if(value < 0){
		DPRINT(1,"caldac set out of range (%d<0)\n", value);
		value = 0;
	}
	dac = &setup->caldacs[ caldac_index ];
	if(value > caldac_maxdata(setup->dev, dac)) {
		DPRINT(1,"caldac set out of range (%d>%d)\n",
			value, caldac_maxdata(setup->dev, dac));
		value = caldac_maxdata(setup->dev, dac);
	}
	dac->value = value;
	DPRINT(4,"update %d %d %d\n", dac->subdevice, dac->channel, dac->value);

	ret = comedi_data_write(setup->dev, dac->subdevice, dac->channel, 0, 0,
		dac->value);
	if(ret < 0) perror("update_caldac()");
}

#if 0
void check_gain(int ad_chan,int range)
{
	int i;

	for(i=0;i<n_caldacs;i++){
		check_gain_chan(ad_chan,range,i);
	}
}
#endif

#if 0
double check_gain_chan(int ad_chan,int range,int cdac)
{
	linear_fit_t l;

	return check_gain_chan_x(&l,CR_PACK(ad_chan,range,AREF_OTHER),cdac);
}
#endif


double check_gain_chan_x( calibration_setup_t *setup, linear_fit_t *l,unsigned int ad_chanspec,int cdac)
{
	int orig,i,n;
	int step;
	new_sv_t sv;
	double sum_err;
	int sum_err_count=0;
	char str[100];

	n = caldac_maxdata(setup->dev, &setup->caldacs[cdac]) + 1;
	memset(l,0,sizeof(*l));

	step=n/16;
	if(step<1)step=1;
	l->n=0;

	l->y_data=malloc(n*sizeof(double)/step);
	if(l->y_data == NULL)
	{
		perror( __FUNCTION__ );
		exit(1);
	}

	orig = setup->caldacs[cdac].value;

	my_sv_init(&sv, setup, setup->ad_subdev,ad_chanspec);

	update_caldac( setup, cdac, 0 );
	usleep(caldac_settle_usec);

	new_sv_measure( setup->dev, &sv);

	sum_err=0;
	for(i=0;i*step<n;i++){
		update_caldac( setup, cdac, i*step );
		usleep(caldac_settle_usec);

		new_sv_measure( setup->dev, &sv);

		l->y_data[i]=sv.average;
		if(!isnan(sv.average)){
			sum_err+=sv.error;
			sum_err_count++;
		}
		l->n++;
	}

	update_caldac( setup, cdac, orig );

	l->yerr=sum_err/sqrt(sum_err_count);
	l->dx=step;
	l->x0=0;

	linear_fit_monotonic(l);

	if(verbose>=2 || (verbose>=1 && (fabs(l->slope / l->err_slope) > 4.0 || isnan(l->slope / l->err_slope)))){
		sci_sprint_alt(str,l->slope,l->err_slope);
		printf("caldac[%d] gain=%s V/bit S_min=%g dof=%g\n",
			cdac,str,l->S_min,l->dof);
		//printf("--> %g\n",fabs(l.slope/l.err_slope));
	}

	if(verbose>=3)dump_curve(l);

	free(l->y_data);

	return l->slope;
}


double check_gain_chan_fine( calibration_setup_t *setup, linear_fit_t *l,unsigned int ad_chanspec,int cdac)
{
	int orig,i,n;
	int step;
	new_sv_t sv;
	double sum_err;
	int sum_err_count=0;
	char str[100];
	int fine_size = 10;

	n=2*fine_size+1;
	memset(l,0,sizeof(*l));

	step=1;
	l->n=0;

	l->y_data=malloc(n*sizeof(double)/step);
	if(l->y_data == NULL)
	{
		perror( __FUNCTION__ );
		exit(1);
	}

	orig = setup->caldacs[cdac].value;

	my_sv_init(&sv, setup, setup->ad_subdev,ad_chanspec);

	update_caldac( setup, cdac, 0 );
	usleep(caldac_settle_usec);

	new_sv_measure( setup->dev, &sv);

	sum_err=0;
	for(i=0;i<n;i++){
		update_caldac( setup, cdac, i+orig-fine_size );
		usleep(caldac_settle_usec);

		new_sv_measure( setup->dev, &sv);

		l->y_data[i]=sv.average;
		if(!isnan(sv.average)){
			sum_err+=sv.error;
			sum_err_count++;
		}
		l->n++;
	}

	update_caldac( setup, cdac, orig );

	l->yerr=sum_err/sqrt(sum_err_count);
	l->dx=1;
	l->x0=orig-fine_size;

	linear_fit_monotonic(l);

	if(verbose>=2 || (verbose>=1 && fabs(l->slope/l->err_slope)>4.0)){
		sci_sprint_alt(str,l->slope,l->err_slope);
		printf("caldac[%d] gain=%s V/bit S_min=%g dof=%g\n",
			cdac,str,l->S_min,l->dof);
		//printf("--> %g\n",fabs(l.slope/l.err_slope));
	}

	if(verbose>=3)dump_curve(l);

	free(l->y_data);

	return l->slope;
}



/* helpers */

int is_unipolar( comedi_t *dev, unsigned int subdevice,
	unsigned int channel, unsigned int range )
{
	comedi_range *range_ptr;

	range_ptr = comedi_get_range( dev, subdevice, channel, range );
	assert( range_ptr != NULL );
	/* This method is better than a direct test, which might fail */
	if( fabs( range_ptr->min ) < fabs( range_ptr->max * 0.001 ) )
		return 1;
	else
		return 0;
}

int is_bipolar( comedi_t *dev, unsigned int subdevice,
	unsigned int channel, unsigned int range )
{
	comedi_range *range_ptr;

	range_ptr = comedi_get_range( dev, subdevice, channel, range );
	assert( range_ptr != NULL );
	/* This method is better than a direct test, which might fail */
	if( fabs( range_ptr->max + range_ptr->min ) < fabs( range_ptr->max * 0.001 ) )
		return 1;
	else
		return 0;
}

int get_bipolar_lowgain(comedi_t *dev,int subdev)
{
	int ret = -1;
	int i;
	int n_ranges = comedi_get_n_ranges(dev,subdev,0);
	double max = 0;
	comedi_range *range;

	for(i=0;i<n_ranges;i++){
		range = comedi_get_range(dev,subdev,0,i);
		if( is_bipolar( dev, subdev, 0, i ) == 0 ) continue;
		if(range->unit == UNIT_none ) continue;
		if(range->max>max){
			ret = i;
			max=range->max;
		}
	}

	return ret;
}

int get_bipolar_highgain(comedi_t *dev,int subdev)
{
	int ret = -1;
	int i;
	int n_ranges = comedi_get_n_ranges(dev,subdev,0);
	double min = HUGE_VAL;
	comedi_range *range;

	for(i=0;i<n_ranges;i++){
		range = comedi_get_range(dev,subdev,0,i);
		if( is_bipolar( dev, subdev, 0, i ) == 0 ) continue;
		if(range->unit == UNIT_none ) continue;
		if(range->max<min){
			ret = i;
			min=range->max;
		}
	}

	return ret;
}

int get_unipolar_lowgain(comedi_t *dev,int subdev)
{
	int ret = -1;
	int i;
	int n_ranges = comedi_get_n_ranges(dev,subdev,0);
	double max = 0.0;
	comedi_range *range;

	for(i=0;i<n_ranges;i++){
		range = comedi_get_range(dev,subdev,0,i);
		if( is_unipolar( dev, subdev, 0, i ) == 0 ) continue;
		if(range->unit == UNIT_none ) continue;
		if(range->max>max){
			ret = i;
			max=range->max;
		}
	}

	return ret;
}

int get_unipolar_highgain(comedi_t *dev,int subdev)
{
	int ret = -1;
	int i;
	int n_ranges = comedi_get_n_ranges(dev,subdev,0);
	double max = HUGE_VAL;
	comedi_range *range;

	for(i=0;i<n_ranges;i++){
		range = comedi_get_range(dev,subdev,0,i);
		if( is_unipolar( dev, subdev, 0, i ) == 0 ) continue;
		if(range->unit == UNIT_none ) continue;
		if(range->max < max){
			ret = i;
			max=range->max;
		}
	}

	return ret;
}

int read_eeprom( calibration_setup_t *setup, int addr)
{
	lsampl_t data = 0;
	int retval;

	retval = comedi_data_read( setup->dev, setup->eeprom_subdev, addr,0,0,&data);
	if( retval < 0 )
	{
		perror( "read_eeprom()" );
		return retval;
	}

	return data;
}

double read_chan( calibration_setup_t *setup, int adc,int range)
{
	int n;
	new_sv_t sv;
	char str[100];

	my_sv_init(&sv, setup,  setup->ad_subdev,CR_PACK(adc,range,AREF_OTHER));

	n=new_sv_measure( setup->dev, &sv);

	sci_sprint_alt(str,sv.average,sv.error);
	printf("chan=%d ave=%s\n",adc,str);

	return sv.average;
}

int read_chan2( calibration_setup_t *setup, char *s,int adc,int range)
{
	int n;
	new_sv_t sv;

	my_sv_init(&sv, setup, setup->ad_subdev,CR_PACK(adc,range,AREF_OTHER));

	n=new_sv_measure( setup->dev, &sv);

	return sci_sprint_alt(s,sv.average,sv.error);
}

#if 0
void set_ao(comedi_t *dev,int subdev,int chan,int range,double value)
{
	comedi_range *r = comedi_get_range(dev,subdev,chan,range);
	lsampl_t maxdata = comedi_get_maxdata(dev,subdev,chan);
	lsampl_t data;

	data = comedi_from_phys(value,r,maxdata);

	comedi_data_write(dev,subdev,chan,range,AREF_GROUND,data);
}
#endif

int my_sv_init( new_sv_t *sv, const calibration_setup_t *setup, int subdev,
	unsigned int chanspec )
{
	int retval;
	retval = new_sv_init( sv, setup->dev, subdev, chanspec );
	sv->settling_time_ns = setup->sv_settling_time_ns;
	sv->order = setup->sv_order;
	return retval;
}

int new_sv_init(new_sv_t *sv,comedi_t *dev,int subdev,unsigned int chanspec)
{
	memset(sv,0,sizeof(*sv));

	sv->subd=subdev;
	//sv->t.flags=TRIG_DITHER;
	sv->chanspec = chanspec;

	//sv->chanlist[0]=CR_PACK(chan,range,aref);

	sv->maxdata=comedi_get_maxdata(dev,subdev,CR_CHAN(chanspec));
	sv->rng=comedi_get_range(dev,subdev,
		CR_CHAN(chanspec), CR_RANGE(chanspec));

	sv->order=10;

	return 0;
}

int new_sv_measure( comedi_t *dev, new_sv_t *sv)
{
	lsampl_t *data;
	int n,i,ret;

	double x,s,s2;

	n=1<<sv->order;

	data=malloc(sizeof(lsampl_t)*n);
	if(data == NULL)
	{
		perror( __FUNCTION__ );
		exit(1);
	}

	ret = comedi_data_read_hint(dev, sv->subd, sv->chanspec, 0, 0);
	if(ret<0){
		printf("hint barf\n");
		goto out;
	}
	comedi_nanodelay(dev, sv->settling_time_ns);

	ret = comedi_data_read_n(dev, sv->subd, sv->chanspec, 0, 0, data, n);
	if(ret<0){
		printf("barf\n");
		goto out;
	}

	s=0.0;
	s2=0.0;
	for(i = 0; i < n; i++){
		x = comedi_to_phys(data[i], sv->rng, sv->maxdata);
		s += x;
		s2 += x * x;
	}
	s /= n;
	s2 /= n;
	sv->average=s;
	sv->stddev=sqrt( ( ( n + 1 ) / n ) * ( s2 - s * s ) );
	sv->error=sv->stddev / sqrt( n );

	ret=n;

out:
	free(data);

	return ret;
}

int new_sv_measure_order( comedi_t *dev, new_sv_t *sv,int order)
{
	lsampl_t *data;
	int n,i,ret;
	double x,s,s2;

	n=1<<order;

	data=malloc(sizeof(lsampl_t)*n);
	if(data == NULL)
	{
		perror( __FUNCTION__ );
		exit(1);
	}

	ret = comedi_data_read_n(dev, sv->subd, sv->chanspec, 0, 0, data, n);
	if(ret<0){
		printf("barf order\n");
		goto out;
	}

	s=0;
	s2=0;
	for(i = 0; i < n; i++){
		x = comedi_to_phys(data[i], sv->rng, sv->maxdata);
		s += x;
		s2 += x * x;
	}
	s /= n;
	s2 /= n;
	sv->average = s;
	sv->stddev=sqrt( ( ( n + 1 ) / n ) * ( s2 - s * s ) );
	sv->error=sv->stddev / sqrt( n );

	ret=n;

out:
	free(data);

	return ret;
}



/* linear fitting */

int calculate_residuals(linear_fit_t *l);

int linear_fit_monotonic(linear_fit_t *l)
{
	double x,y;
	double sxp;
	int i;

	l->min=HUGE_VAL;
	l->max=-HUGE_VAL;
	l->s1=0;
	l->sx=0;
	l->sy=0;
	l->sxy=0;
	l->sxx=0;
	for(i=0;i<l->n;i++){
		x=l->x0+i*l->dx;
		y=l->y_data[i];

		if(isnan(y))continue;

		if(l->y_data[i]<l->min)l->min=l->y_data[i];
		if(l->y_data[i]>l->max)l->max=l->y_data[i];
		l->s1+=1;
		l->sx+=x;
		l->sy+=y;
		l->sxy+=x*y;
		l->sxx+=x*x;
	}
	sxp=l->sxx-l->sx*l->sx/l->s1;

	l->ave_x=l->sx/l->s1;
	l->ave_y=l->sy/l->s1;
	l->slope=(l->s1*l->sxy-l->sx*l->sy)/(l->s1*l->sxx-l->sx*l->sx);
	l->err_slope=l->yerr/sqrt(sxp);
	l->err_ave_y=l->yerr/sqrt(l->s1);

	calculate_residuals(l);

	return 0;
}

int calculate_residuals(linear_fit_t *l)
{
	double x,y;
	double res,sum_res2;
	int i;

	sum_res2=0;
	for(i=0;i<l->n;i++){
		x=l->x0+i*l->dx-l->ave_x;
		y=l->y_data[i];

		if(isnan(y))continue;

		res=l->ave_y+l->slope*x-y;

		sum_res2+=res*res;
	}
	l->S_min=sum_res2/(l->yerr*l->yerr);
	l->dof=l->s1-2;

	return 0;
}

double linear_fit_func_y(linear_fit_t *l,double x)
{
	return l->ave_y+l->slope*(x-l->ave_x);
}

double linear_fit_func_x(linear_fit_t *l,double y)
{
	return l->ave_x+(y-l->ave_y)/l->slope;
}

void dump_curve(linear_fit_t *l)
{
	static int dump_number=0;
	double x,y;
	int i;

	printf("start dump %d\n",dump_number);
	for(i=0;i<l->n;i++){
		x=l->x0+i*l->dx-l->ave_x;
		y=l->y_data[i];
		printf("D%d: %d %g %g %g\n",dump_number,i,y,
			l->ave_y+l->slope*x,
			l->ave_y+l->slope*x-y);
	}
	printf("end dump\n");
	dump_number++;
}


/* printing of scientific numbers (with errors) */

int sci_sprint(char *s,double x,double y)
{
	int errsig;
	int maxsig;
	int sigfigs;
	double mantissa;
	double error;
	double mindigit;

	errsig = floor(log10(y));
	maxsig = floor(log10(x));
	mindigit = pow(10,errsig);

	if(maxsig<errsig)maxsig=errsig;

	sigfigs = maxsig-errsig+2;

	mantissa = x*pow(10,-maxsig);
	error = y*pow(10,-errsig+1);

	return sprintf(s,"%0.*f(%2.0f)e%d",sigfigs-1,mantissa,error,maxsig);
}

int sci_sprint_alt(char *s,double x,double y)
{
	int errsig;
	int maxsig;
	int sigfigs;
	double mantissa;
	double error;
	double mindigit;

	errsig = floor(log10(fabs(y)));
	maxsig = floor(log10(fabs(x)));
	mindigit = pow(10,errsig);

	if(maxsig<errsig)maxsig=errsig;
	sigfigs = maxsig-errsig+2;

	mantissa = x*pow(10,-maxsig);
	error = y*pow(10,-errsig+1);

	if(isnan(x) || isnan(y)){
		return sprintf(s,"%g",x);
	}
	if(errsig==1 && maxsig<4 && maxsig>1){
		return sprintf(s,"%0.0f(%2.0f)",x,error);
	}
	if(maxsig<=0 && maxsig>=-2){
		return sprintf(s,"%0.*f(%2.0f)",sigfigs-1-maxsig,
			mantissa*pow(10,maxsig),error);
	}
	return sprintf(s,"%0.*f(%2.0f)e%d",sigfigs-1,mantissa,error,maxsig);
}

double very_low_target( comedi_t *dev, unsigned int subdevice,
	unsigned int channel, unsigned int range )
{
	comedi_range *range_ptr;
	int max_data;

	range_ptr = comedi_get_range( dev, subdevice, channel, range );
	assert( range_ptr != NULL );
	max_data = comedi_get_maxdata( dev, subdevice, 0 );
	assert( max_data > 0 );

	return comedi_to_phys( 1, range_ptr, max_data ) / 2.0;
}

double fractional_offset( calibration_setup_t *setup, int subdevice,
	unsigned int channel, unsigned int range, int obs )
{
	comedi_range *range_ptr;
	double target;
	double reading;
	unsigned int chanspec;
	new_sv_t sv;

	if( subdevice < 0 || obs < 0 ) return 0.0;

	chanspec = setup->observables[obs].observe_insn.chanspec;
	target = setup->observables[obs].target;

	range_ptr = comedi_get_range( setup->dev, subdevice, channel, range );
	assert( range_ptr != NULL );

	comedi_set_global_oor_behavior( COMEDI_OOR_NUMBER );
	preobserve( setup, obs);

	my_sv_init( &sv, setup, setup->ad_subdev, chanspec );
	new_sv_measure( setup->dev, &sv );
	reading = sv.average;

	return ( reading - target ) / ( range_ptr->max - range_ptr->min );
}

double get_tolerance( calibration_setup_t *setup, int subdevice,
	double num_bits )
{
	int maxdata;

	if( subdevice < 0 ) return INFINITY;

	maxdata = comedi_get_maxdata( setup->dev, subdevice, 0 );
	assert( maxdata > 0 );
	return num_bits / maxdata;
}

unsigned caldac_maxdata(comedi_t *dev, const comedi_caldac_t *caldac)
{
	unsigned maxdata = comedi_get_maxdata(dev, caldac->subdevice, caldac->channel);
	assert(maxdata > 0);
	return maxdata;
}
