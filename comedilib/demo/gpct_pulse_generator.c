/*
 * NI general-purpose counter example.  Configures the counter to
 * produce a continuous pulse train.  The argument specifies the
 * number of nanoseconds the output pulse should remain high.
 * The example assumes the board has already been configured to
 * route the output signal of the counter to an appropriate
 * location (you may need to route it to a PFI output line
 * for example).  The choose_routing demo may be helpful to
 * set the routing of the counter's output signal.
 * Part of Comedilib
 *
 * Copyright (c) 2007 Frank Mori Hess <fmhess@users.sourceforge.net>
 *
 * This file may be freely modified, distributed, and combined with
 * other software, as long as proper attribution is given in the
 * source code.
 */
/*
 * Requirements:  A board with a National Instruments general-purpose
 * counter, and comedi driver version 0.7.74 or newer.
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <comedilib.h>
#include <math.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <getopt.h>
#include <ctype.h>
#include "examples.h"


int ni_gpct_start_pulse_generator(comedi_t *device, unsigned subdevice, unsigned period_ns, unsigned up_time_ns)
{
	int retval;
	lsampl_t counter_mode;
	const unsigned clock_period_ns = 50;	/* 20MHz clock */
	unsigned up_ticks, down_ticks;

	retval = comedi_reset(device, subdevice);
	if(retval < 0) return retval;

	retval = comedi_set_gate_source(device, subdevice, 0, 0, NI_GPCT_DISABLED_GATE_SELECT | CR_EDGE);
	if(retval < 0) return retval;
	retval = comedi_set_gate_source(device, subdevice, 0, 1, NI_GPCT_DISABLED_GATE_SELECT | CR_EDGE);
	if(retval < 0)
	{
		fprintf(stderr, "Failed to set second gate source.  This is expected for older boards (e-series, etc.)\n"
			"that don't have a second gate.\n");
	}

	counter_mode = NI_GPCT_COUNTING_MODE_NORMAL_BITS;
	// toggle output on terminal count
	counter_mode |= NI_GPCT_OUTPUT_TC_TOGGLE_BITS;
	// load on terminal count
	counter_mode |= NI_GPCT_LOADING_ON_TC_BIT;
	// alternate the reload source between the load a and load b registers
	counter_mode |= NI_GPCT_RELOAD_SOURCE_SWITCHING_BITS;
	// count down
	counter_mode |= NI_GPCT_COUNTING_DIRECTION_DOWN_BITS;
	// initialize load source as load b register
	counter_mode |= NI_GPCT_LOAD_B_SELECT_BIT;
	// don't stop on terminal count
	counter_mode |= NI_GPCT_STOP_ON_GATE_BITS;
	// don't disarm on terminal count or gate signal
	counter_mode |= NI_GPCT_NO_HARDWARE_DISARM_BITS;
	retval = comedi_set_counter_mode(device, subdevice, 0, counter_mode);
	if(retval < 0) return retval;

	/* 20MHz clock */
	retval = comedi_set_clock_source(device, subdevice, NI_GPCT_TIMEBASE_1_CLOCK_SRC_BITS, clock_period_ns);
	if(retval < 0) return retval;

	up_ticks = (up_time_ns + clock_period_ns / 2) / clock_period_ns;
	down_ticks = (period_ns + clock_period_ns / 2) / clock_period_ns - up_ticks;
	/* set initial counter value by writing to channel 0 */
	retval = comedi_data_write(device, subdevice, 0, 0, 0, down_ticks);
	if(retval < 0) return retval;
	/* set "load a" register to the number of clock ticks the counter output should remain low
	by writing to channel 1. */
	comedi_data_write(device, subdevice, 1, 0, 0, down_ticks);
	if(retval < 0) return retval;
	/* set "load b" register to the number of clock ticks the counter output should remain high
	by writing to channel 2 */
	comedi_data_write(device, subdevice, 2, 0, 0, up_ticks);
	if(retval < 0) return retval;

	retval = comedi_arm(device, subdevice, NI_GPCT_ARM_IMMEDIATE);
	if(retval < 0) return retval;

	return 0;
}

int main(int argc, char *argv[])
{
	comedi_t *device;
	unsigned up_time;
	unsigned period_ns;
	int retval;
	int subdevice_type;
	struct parsed_options options;

	init_parsed_options(&options);
	options.value = -0.5;
	parse_options(&options, argc, argv);
	period_ns = lrint(1e9 / options.freq);
	if(options.value < 0.)
	{
		double duty;
		if(-options.value < 1.0)
			duty = -options.value;
		else
			duty = 1.0 / -options.value;
		up_time = lrint(duty * period_ns);
	}
	else
		up_time = lrint(options.value);
	device = comedi_open(options.filename);
	if(!device)
	{
		comedi_perror(options.filename);
		exit(-1);
	}
	subdevice_type = comedi_get_subdevice_type(device, options.subdevice);
	if(subdevice_type < 0)
	{
		comedi_perror("comedi_get_subdevice_type()");
		return -1;
	}
	if(subdevice_type != COMEDI_SUBD_COUNTER)
	{
		fprintf(stderr, "Subdevice is not a counter (type %i), but of type %i.\n",
			COMEDI_SUBD_COUNTER, subdevice_type);
		return -1;
	}
	printf("Generating pulse train on subdevice %d.\n", options.subdevice);
	printf("Period = %d ns.\n", period_ns);
	printf("Up Time = %d ns.\n", up_time);
	printf("Down Time = %d ns.\n", period_ns - up_time);

	retval = ni_gpct_start_pulse_generator(device, options.subdevice, period_ns, up_time);
	if(retval < 0) return retval;
	return 0;
}

