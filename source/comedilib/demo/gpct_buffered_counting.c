/*
 * NI general-purpose counter example.  Makes the counter
 * do buffered event counting using comedi_command().
 * Different clock sources can be chosen
 * with the choose_clock demo.  The gate source is
 * set to PFI1 (which is assumed to already be configured as
 * an input).  The gate source latches the count
 * into the data stream, and possibly resets it (for
 * non-cumulative buffered counting).
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

#include <assert.h>
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

int check_subdevice(comedi_t *device, int *subdevice, const char *device_filepath)
{
	int subdevice_type;
	int read_subdevice = comedi_get_read_subdevice(device);
	if(read_subdevice < 0)
	{
		fprintf(stderr, "Device file \"%s\" cannot do streaming input.\n", device_filepath);
		return -1;
	}
	if(*subdevice < 0) *subdevice = read_subdevice;
	if(read_subdevice != *subdevice)
	{
		fprintf(stderr, "You specified subdevice %i, but the read subdevice of device file \"%s\" is %i.\n",
			*subdevice, device_filepath, read_subdevice);
		return -1;
	}
	subdevice_type = comedi_get_subdevice_type(device, *subdevice);
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
	return 0;
}

int ni_gpct_configure_buffered_event_counting(comedi_t *device, unsigned subdevice)
{
	int retval;
	lsampl_t counter_mode;
	static const unsigned gate_pfi_channel = 1;
	static const unsigned initial_count = 0;

	retval = comedi_reset(device, subdevice);
	if(retval < 0) return retval;

#if 1
	// PFI gate select works for e and m series
	retval = comedi_set_gate_source(device, subdevice, 0, 0, NI_GPCT_PFI_GATE_SELECT(gate_pfi_channel) /*| CR_EDGE*/);
#else
	// gate pin gate select works for 660x
	retval = comedi_set_gate_source(device, subdevice, 0, 0, NI_GPCT_GATE_PIN_i_GATE_SELECT /*| CR_EDGE*/);
#endif
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
	/* Don't alternate the reload source between the load a and load b registers.*/
	counter_mode |= NI_GPCT_RELOAD_SOURCE_FIXED_BITS;
	/* Reload counter on gate.  Do not set if you want to do cumulative buffered counting. */
	counter_mode |= NI_GPCT_LOADING_ON_GATE_BIT;
	/* Make gate start/stop counting */
	counter_mode |= NI_GPCT_EDGE_GATE_STARTS_STOPS_BITS;
	// count up
	counter_mode |= NI_GPCT_COUNTING_DIRECTION_UP_BITS;
	// don't stop on terminal count
	counter_mode |= NI_GPCT_STOP_ON_GATE_BITS;
	// don't disarm on terminal count or gate signal
	counter_mode |= NI_GPCT_NO_HARDWARE_DISARM_BITS;
	retval = comedi_set_counter_mode(device, subdevice, 0, counter_mode);
	if(retval < 0) return retval;

	/* Set initial counter value by writing to channel 0.*/
	retval = comedi_data_write(device, subdevice, 0, 0, 0, initial_count);
	/* Set value of "load a" register by writing to channel 1.*/
	retval = comedi_data_write(device, subdevice, 1, 0, 0, initial_count);

	return 0;
}

int ni_gpct_send_command(comedi_t *device, unsigned subdevice, unsigned n_counts)
{
	comedi_cmd cmd;
	static const unsigned num_chan = 1;
	lsampl_t chanlist[num_chan];
	int retval;

	memset(&cmd, 0, sizeof(cmd));
	cmd.subdev = subdevice;
	cmd.flags = 0;
	/* Wake up at the end of every scan, reduces latency for slow data streams.
	Turn off for more efficient throughput. */
	cmd.flags |= TRIG_WAKE_EOS;
	cmd.start_src = TRIG_NOW;
	cmd.start_arg = 0;
	/* TRIG_OTHER will use the already configured gate source to latch counts.  Can also use
	TRIG_EXT and specify the gate source as the scan_begin_arg */
	cmd.scan_begin_src = TRIG_OTHER;
	cmd.scan_begin_arg = 0;
	cmd.convert_src =	TRIG_NOW;
	cmd.convert_arg = 0;
	cmd.scan_end_src =	TRIG_COUNT;
	cmd.scan_end_arg = num_chan;
	cmd.stop_src = TRIG_NONE;
	cmd.stop_arg = 0;
	cmd.chanlist = chanlist;
	cmd.chanlist_len = num_chan;
	chanlist[0] = 0;

	retval = comedi_command_test(device, &cmd);
	if(retval)
	{
		fprintf(stderr, "comedi_command_test() failed.\n");
		return -1;
	}
	retval = comedi_command(device, &cmd);
	if(retval)
	{
		fprintf(stderr, "comedi_command() failed.\n");
		return -1;
	}
	return 0;
}

int ni_gpct_read_and_dump_counts(comedi_t *device, unsigned subdevice)
{
	int retval;
	int fd;
	static const unsigned buffer_size = 1000;
	lsampl_t buffer[buffer_size];

	fd = comedi_fileno(device);
	retval = read(fd, buffer, buffer_size * sizeof(lsampl_t));
	while(retval > 0)
	{
		unsigned num_counts = retval / sizeof(lsampl_t);
		unsigned i;
		for(i = 0; i < num_counts; ++i)
		{
			printf("%i\n", buffer[i]);
		}
		retval = read(fd, buffer, buffer_size * sizeof(lsampl_t));
	}
	if(retval < 0)
	{
		fprintf(stderr, "error reading from subdevice file.\n");
		perror("read");
		return -errno;
	}
	return 0;
}

int main(int argc, char *argv[])
{
	comedi_t *device;
	int retval;
	struct parsed_options options;

	init_parsed_options(&options);
	options.subdevice = -1;
	parse_options(&options, argc, argv);
	device = comedi_open(options.filename);
	if(!device)
	{
		comedi_perror(options.filename);
		exit(-1);
	}
	retval = check_subdevice(device, &options.subdevice, options.filename);
	if(retval < 0) return retval;
	printf("Running buffered event counting on subdevice %d.\n", options.subdevice);

	retval = ni_gpct_configure_buffered_event_counting(device, options.subdevice);
	if(retval < 0) return retval;
	retval = ni_gpct_send_command(device, options.subdevice, options.n_scan);
	if(retval < 0) return retval;
	retval = ni_gpct_read_and_dump_counts(device, options.subdevice);
	if(retval < 0) return retval;
	return 0;
}

