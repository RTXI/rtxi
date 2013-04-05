/*
 * Multi-channel, multi-range one-shot input demo
 * Part of Comedilib
 *
 * Copyright (c) 1999,2000 David A. Schleef <ds@schleef.org>
 *
 * This file may be freely modified, distributed, and combined with
 * other software, as long as proper attribution is given in the
 * source code.
 */
/*
   This demo opens /dev/comedi0 and looks for an analog input
   subdevice.  If it finds one, it measures one sample on each
   channel for each input range.  The value NaN indicates that
   the measurement was out of range.
 */

#include <stdio.h>
#include <comedilib.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <getopt.h>
#include <ctype.h>
#include "examples.h"

comedi_t *device;


int main(int argc, char *argv[])
{
	int n_chans,chan;
	int n_ranges;
	int range;
	int maxdata;
	lsampl_t data;
	double voltage;
	struct parsed_options options;

	init_parsed_options(&options);
	options.subdevice = -1;
	parse_options(&options, argc, argv);

	device = comedi_open(options.filename);
	if(!device){
		comedi_perror(options.filename);
		exit(-1);
	}

	if(options.subdevice < 0)
	{
		options.subdevice = comedi_find_subdevice_by_type(device, COMEDI_SUBD_AI, 0);
		if(options.subdevice<0){
			printf("no analog input subdevice found\n");
			exit(-1);
		}
	}
	n_chans = comedi_get_n_channels(device, options.subdevice);
	for(chan = 0; chan < n_chans; ++chan){
		printf("%d: ", chan);

		n_ranges = comedi_get_n_ranges(device, options.subdevice, chan);

		maxdata = comedi_get_maxdata(device, options.subdevice, chan);
		for(range = 0; range < n_ranges; range++){
			comedi_data_read(device, options.subdevice, chan, options.range, options.aref, &data);
			voltage = comedi_to_phys(data, comedi_get_range(device, options.subdevice, chan, options.range), maxdata);
			printf("%g ", voltage);
		}
		printf("\n");
	}
	return 0;
}

