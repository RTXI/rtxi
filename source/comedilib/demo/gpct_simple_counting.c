/*
 * NI general-purpose counter example.  Configures the counter to
 * start simple event counting.  The counter's value can be read
 * with the inp demo.  Different clock sources can be chosen
 * with the choose_clock demo.
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

int ni_gpct_start_simple_event_counting(comedi_t *device, unsigned subdevice)
{
	int retval;
	lsampl_t counter_mode;
	static const unsigned initial_count = 0;

	retval = comedi_reset(device, subdevice);
	if(retval < 0) return retval;

	retval = comedi_set_gate_source(device, subdevice, 0, 0, NI_GPCT_GATE_PIN_GATE_SELECT(0) /* NI_GPCT_GATE_PIN_i_GATE_SELECT *//*| CR_EDGE*/);
	if(retval < 0) return retval;
	retval = comedi_set_gate_source(device, subdevice, 0, 1, NI_GPCT_DISABLED_GATE_SELECT | CR_EDGE);
	if(retval < 0)
	{
		fprintf(stderr, "Failed to set second gate source.  This is expected for older boards (e-series, etc.)\n"
			"that don't have a second gate.\n");
	}

	counter_mode = NI_GPCT_COUNTING_MODE_NORMAL_BITS;
	// output pulse on terminal count (doesn't really matter for this application)
	counter_mode |= NI_GPCT_OUTPUT_TC_PULSE_BITS;
	/* Don't alternate the reload source between the load a and load b registers.
		Doesn't really matter here, since we aren't going to be reloading the counter.
	*/
	counter_mode |= NI_GPCT_RELOAD_SOURCE_FIXED_BITS;
	// count up
	counter_mode |= NI_GPCT_COUNTING_DIRECTION_UP_BITS;
	// don't stop on terminal count
	counter_mode |= NI_GPCT_STOP_ON_GATE_BITS;
	// don't disarm on terminal count or gate signal
	counter_mode |= NI_GPCT_NO_HARDWARE_DISARM_BITS;
	retval = comedi_set_counter_mode(device, subdevice, 0, counter_mode);
	if(retval < 0) return retval;

	/* set initial counter value by writing to channel 0.  The "load a" and "load b" registers can be
	set by writing to channels 1 and 2 respectively. */
	retval = comedi_data_write(device, subdevice, 0, 0, 0, initial_count);

	retval = comedi_arm(device, subdevice, NI_GPCT_ARM_IMMEDIATE);
	if(retval < 0) return retval;

	return 0;
}

int main(int argc, char *argv[])
{
	comedi_t *device;
	int retval;
	struct parsed_options options;

	init_parsed_options(&options);
	parse_options(&options, argc, argv);
	device = comedi_open(options.filename);
	if(!device)
	{
		comedi_perror(options.filename);
		exit(-1);
	}
	/*FIXME: check that device is counter */
	printf("Initiating simple event counting on subdevice %d.\n", options.subdevice);

	retval = ni_gpct_start_simple_event_counting(device, options.subdevice);
	if(retval < 0) return retval;
	return 0;
}

