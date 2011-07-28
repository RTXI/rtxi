/*
 * Demo of the comedi_sv_*() functions
 *
 * Part of Comedilib
 *
 * Copyright (c) 1999,2000 David A. Schleef <ds@schleef.org>
 *
 * This file may be freely modified, distributed, and combined with
 * other software, as long as proper attribution is given in the
 * source code.
 */

#include <stdio.h>
#include <comedilib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <getopt.h>
#include <ctype.h>
#include <stdlib.h>
#include "examples.h"

comedi_t *device;


int main(int argc, char *argv[])
{
	int ret;
	comedi_sv_t sv;
	double volts;
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

	comedi_sv_init(&sv, device, options.subdevice, options.channel);

	sv.range = options.range;
	sv.aref = options.aref;
	sv.n = 100;

	ret = comedi_sv_measure(&sv, &volts);
	if(ret < 0){
		comedi_perror("comedi_sv_measure()");
		exit(-1);
	}

	printf("%g\n", volts);

	return 0;
}

