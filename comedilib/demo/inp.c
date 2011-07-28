/*
 * A very small one-shot input demo
 * Part of Comedilib
 *
 * Copyright (c) 1999,2000 David A. Schleef <ds@schleef.org>
 *
 * This file may be freely modified, distributed, and combined with
 * other software, as long as proper attribution is given in the
 * source code.
 */
/*
   A little input demo
 */

#include <stdio.h>
#include <comedilib.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <getopt.h>
#include <ctype.h>
#include <math.h>
#include "examples.h"

comedi_t *device;


int main(int argc, char *argv[])
{
	lsampl_t data;
	int ret;
	comedi_range * range_info;
	lsampl_t maxdata;
	double physical_value;
	struct parsed_options options;

	init_parsed_options(&options);
	parse_options(&options, argc, argv);

	device = comedi_open(options.filename);
	if(!device){
		comedi_perror(options.filename);
		exit(-1);
	}

	if(options.verbose){
		printf("measuring device=%s subdevice=%d channel=%d range=%d analog reference=%d\n",
			options.filename, options.subdevice, options.channel, options.range, options.aref);
	}

	ret = comedi_data_read(device, options.subdevice, options.channel, options.range, options.aref, &data);
	if(ret < 0){
		comedi_perror(options.filename);
		exit(-1);
	}

	if(options.physical) {
		comedi_set_global_oor_behavior(COMEDI_OOR_NAN);
		range_info = comedi_get_range(device, options.subdevice, options.channel, options.range);
		maxdata = comedi_get_maxdata(device, options.subdevice, options.channel);
		if(options.verbose) {
			printf("[0,%d] -> [%g,%g]\n", maxdata,
				range_info->min, range_info->max);
		}
		physical_value = comedi_to_phys(data, range_info, maxdata);
		if(isnan(physical_value)) {
			printf("Out of range [%g,%g]",
				range_info->min, range_info->max);
		} else {
			printf("%g", physical_value);
			switch(range_info->unit) {
				case UNIT_volt: printf(" V"); break;
				case UNIT_mA: printf(" mA"); break;
				case UNIT_none: break;
				default: printf(" (unknown unit %d)",
					range_info->unit);
			}
			if(options.verbose) {
				printf(" (%lu raw units)", (unsigned long)data);
			}
		}
	} else {
		printf("%lu", (unsigned long)data);
	}
	printf("\n");

	return 0;
}

