/*
 * A little output demo
 * Part of Comedilib
 *
 * Copyright (c) 1999,2000 David A. Schleef <ds@schleef.org>
 *
 * This file may be freely modified, distributed, and combined with
 * other software, as long as proper attribution is given in the
 * source code.
 */
/*
 * A little output demo
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
	lsampl_t data;
	int ret;
	struct parsed_options options;

	init_parsed_options(&options);
	parse_options(&options, argc, argv);

	device = comedi_open(options.filename);
	if(!device){
		comedi_perror(options.filename);
		exit(-1);
	}

	data = options.value;
	if(options.verbose){
		printf("writing %d to device=%s subdevice=%d channel=%d range=%d analog reference=%d\n",
			data, options.filename, options.subdevice, options.channel, options.range, options.aref);
	}

	ret = comedi_data_write(device, options.subdevice, options.channel, options.range, options.aref, data);
	if(ret < 0){
		comedi_perror(options.filename);
		exit(-1);
	}

	printf("%d\n", data);

	return 0;
}

