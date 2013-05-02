/*
 * INSN_CONFIG_FILTER example
 * Part of Comedilib
 *
 * Copyright (c) 1999,2000 David A. Schleef <ds@schleef.org>
 * Copyright (c) 2007 Frank Mori Hess <fmhess@users.sourceforge.net>
 *
 * This file may be freely modified, distributed, and combined with
 * other software, as long as proper attribution is given in the
 * source code.
 */
/*
 * Requirements:  A board with a subdevice that supports
 *    INSN_CONFIG_FILTER, such as the PFI subdevice on an NI m-series
 *    or 660x board.
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <comedilib.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <getopt.h>
#include <ctype.h>
#include "examples.h"


comedi_t *device;

int main(int argc, char *argv[])
{
	int retval;
	lsampl_t filter_selection;
	struct parsed_options options;

	init_parsed_options(&options);
	parse_options(&options, argc, argv);

	device = comedi_open(options.filename);
	if(!device){
		comedi_perror(options.filename);
		exit(-1);
	}
	filter_selection = options.value;
	printf("Selecting filter %d on subdevice %d channel %d.\n", filter_selection, options.subdevice, options.channel);

	retval = comedi_set_filter(device, options.subdevice, options.channel, filter_selection);
	if(retval < 0) comedi_perror("comedi_do_insn");
	return retval;
}

