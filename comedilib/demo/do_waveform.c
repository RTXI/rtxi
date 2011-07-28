/*
 * Asynchronous Digital Output Example
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
 * Requirements: NI M-Series board.
 *
 * This demo uses the digital io subdevice with an
 * asynchronous command to generate waveforms.  The
 * waveforms in this example are square waves with
 * varying periods depending on the digital output
 * channel.  Channel N outputs a square wave with
 * period 2*(N+1) clocks. The command line argument chooses the
 * clock signal for updating the outputs.  As a suggestion,
 * you might use the output of one of the general
 * purpose counters for a clock (the default clock
 * source is the output of general purpose counter 0),
 * and run the gpct_pulse_generator demo to start the counter
 * generating pulses on its output.
 *
 * Note, you must configure at least one of the digital channels as
 * an output (for example by running the dio demo program)
 * before running this program. You must also pass the dio
 * subdevice file using the -f option, since the default write
 * subdevice for the m-series boards is the analog output.  For
 * example, "dio_waveform -f /dev/comedi0_sub2".
 */

#include <assert.h>
#include <comedilib.h>
#include <ctype.h>
#include <errno.h>
#include "examples.h"
#include <fcntl.h>
#include <malloc.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BUF_LEN 0x1000

struct test_waveform_generator
{
	unsigned *toggle_countdowns;
	unsigned num_channels;
	lsampl_t last_output;
};

int init_test_waveform_generator(struct test_waveform_generator *generator, unsigned num_channels)
{
	unsigned i;

	generator->num_channels = num_channels;
	generator->toggle_countdowns = malloc(sizeof(unsigned) * num_channels);
	if(generator->toggle_countdowns == NULL) return -ENOMEM;
	for(i = 0; i < num_channels; ++i)
		generator->toggle_countdowns[i] = i + 1;
	generator->last_output = 0;
	return 0;
}

void cleanup_test_waveform_generator(struct test_waveform_generator *generator)
{
	if(generator->toggle_countdowns)
	{
		free(generator->toggle_countdowns);
		generator->toggle_countdowns = NULL;
	}
}

void build_waveforms(struct test_waveform_generator *generator, lsampl_t *data, unsigned data_len)
{
	unsigned i;
	for(i = 0; i < data_len; ++i)
	{
		unsigned j;

		data[i] = generator->last_output;
		for(j = 0; j < generator->num_channels; ++j)
		{
			if(--generator->toggle_countdowns[j] == 0)
			{
				generator->toggle_countdowns[j] = j + 1;
				data[i] ^= 1 << j;
			}
		}
		generator->last_output = data[i];
	}
}

int main(int argc, char *argv[])
{
	comedi_cmd cmd;
	struct parsed_options options;
	comedi_t *dev;
	int i;
	struct test_waveform_generator generator;
	lsampl_t *buffer = NULL;
	int err;
	int n, m;
	int total=0;
	unsigned int *chanlist;
	int ret;

	init_parsed_options(&options);
	options.subdevice = -1;
	options.n_chan = 8;
	options.value = NI_CDIO_SCAN_BEGIN_SRC_G0_OUT;
	parse_options(&options, argc, argv);

	dev = comedi_open(options.filename);
	if(dev == NULL){
		fprintf(stderr, "error opening %s\n", options.filename);
		return -1;
	}
	if(options.subdevice < 0)
		options.subdevice = comedi_find_subdevice_by_type(dev, COMEDI_SUBD_DIO, 0);

	memset(&cmd,0,sizeof(cmd));
	cmd.subdev = options.subdevice;
	cmd.flags = CMDF_WRITE;
	cmd.start_src = TRIG_INT;
	cmd.start_arg = 0;
	cmd.scan_begin_src = TRIG_EXT;
	cmd.scan_begin_arg = options.value;
	cmd.convert_src = TRIG_NOW;
	cmd.convert_arg = 0;
	cmd.scan_end_src = TRIG_COUNT;
	cmd.scan_end_arg = options.n_chan;
	cmd.stop_src = TRIG_NONE;
	cmd.stop_arg = 0;

	chanlist = malloc(options.n_chan * sizeof(unsigned));
	assert(chanlist);
	cmd.chanlist = chanlist;
	cmd.chanlist_len = options.n_chan;
	for(i = 0; i < cmd.chanlist_len; ++i)
	{
		cmd.chanlist[i] = i;
	}

	dump_cmd(stdout, &cmd);

	err = comedi_command_test(dev, &cmd);
	if (err < 0) {
		comedi_perror("comedi_command_test");
		exit(1);
	}

	err = comedi_command_test(dev, &cmd);
	if (err < 0) {
		comedi_perror("comedi_command_test");
		exit(1);
	}

	err = comedi_command_test(dev, &cmd);
	if(err != 0) {
		fprintf(stderr, "After 3 passes, comedi_command_test returns %i\n", err);
		exit(1);
	}

	if ((err = comedi_command(dev, &cmd)) < 0) {
		comedi_perror("comedi_command");
		exit(1);
	}

	ret = init_test_waveform_generator(&generator, cmd.chanlist_len);
	assert(ret == 0);
	buffer = malloc(sizeof(lsampl_t) * BUF_LEN);
	assert(buffer);
	build_waveforms(&generator, buffer, BUF_LEN);
	n = BUF_LEN * sizeof(buffer[0]);
	m = write(comedi_fileno(dev), buffer, n);
	if(m < 0){
		perror("write");
		exit(1);
	}else if(m < n)
	{
		fprintf(stderr, "Failed to preload output buffer with %i bytes, is it too small?\n"
			"See the --write-buffer option of comedi_config\n", n);
		exit(1);
	}
	printf("m=%d\n",m);

	ret = comedi_internal_trigger(dev, options.subdevice, 0);
	if(ret < 0){
		perror("comedi_internal_trigger\n");
		exit(1);
	}

	while(1)
	{
		build_waveforms(&generator, buffer, BUF_LEN);
		n = BUF_LEN * sizeof(buffer[0]);
		while(n > 0)
		{
			m = write(comedi_fileno(dev), (void*)(buffer) + (BUF_LEN * sizeof(buffer[0]) - n), n);
			if(m < 0){
				perror("write");
				exit(0);
			}
			printf("m=%d\n",m);
			n -= m;
		}
		total += BUF_LEN;
		//printf("%d\n",total);
	}

	cleanup_test_waveform_generator(&generator);
	free(buffer);
	free(chanlist);
	return 0;
}
