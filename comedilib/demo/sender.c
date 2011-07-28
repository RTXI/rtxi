/*
 * Digital I/O example
 * Part of Comedilib
 *
 * Copyright (c) 1999,2000 David A. Schleef <ds@schleef.org>
 *
 * This file may be freely modified, distributed, and combined with
 * other software, as long as proper attribution is given in the
 * source code.
 */
/*
 * Requirements:  A board with a digital I/O subdevice.  Not just
 *    a 'digital input' or 'digital output' subdevice, but one in
 *    which the channels can be configured between input and output.
 */

#include <stdio.h>
#include <comedilib.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "examples.h"


int chan_dat = 1;
int chan_clk = 0;

int wait1 = usec_to_nsec(0);
int wait2 = usec_to_nsec(0);

comedi_t *device;

void write_bits(int subdevice, int bits);


int main(int argc, char *argv[])
{
	int ret;
	int stype;
	int i;
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
		options.subdevice = comedi_find_subdevice_by_type(device, COMEDI_SUBD_DIO, 0);
		if(options.subdevice < 0){
			fprintf(stderr,"No dio subdevice found.\n");
			exit(-1);
		}
	}
	stype = comedi_get_subdevice_type(device, options.subdevice);
	if(stype != COMEDI_SUBD_DIO){
		printf("%d is not a digital I/O subdevice\n", options.subdevice);
		exit(-1);
	}

	printf("configuring pin %d for output...\n", chan_dat);
	ret = comedi_dio_config(device, options.subdevice, chan_dat, COMEDI_OUTPUT);

	printf("configuring pin %d for output...\n", chan_clk);
	ret = comedi_dio_config(device, options.subdevice, chan_clk, COMEDI_OUTPUT);

	for(i = 0; i < 0x100; i++){
		write_bits(options.subdevice, i);
	}
	//write_bits(0xa5);

	return 0;
}


void write_bits(int subdevice, int bits)
{
	comedi_insnlist il;
	comedi_insn insn[5];
	lsampl_t data[10];
	int mask = (1<<chan_dat)|(1<<chan_clk);
	int i;
	int bit;
	int ret;

	il.n_insns = 5;
	il.insns = insn;

	memset(insn,0,sizeof(insn));

	/* clock low, set data */
	insn[0].insn = INSN_BITS;
	insn[0].n = 2;
	insn[0].data = data + 0;
	insn[0].subdev = subdevice;

	/* wait 1 */
	insn[1].insn = INSN_WAIT;
	insn[1].n = 1;
	insn[1].data = data + 2;

	/* clock high, same data */
	insn[2].insn = INSN_BITS;
	insn[2].n = 2;
	insn[2].data = data + 4;
	insn[2].subdev = subdevice;

	/* wait 1 */
	insn[3].insn = INSN_WAIT;
	insn[3].n = 1;
	insn[3].data = data + 6;

	/* clock low, same data */
	insn[4].insn = INSN_BITS;
	insn[4].n = 2;
	insn[4].data = data + 8;
	insn[4].subdev = subdevice;


	for(i=0;i<8;i++){
		bit=1<<(7-i);
//printf("writing %d\n",bit&bits);

		data[0] = mask;
		data[1] = (bits&bit)?(1<<chan_dat):0;

		data[2] = wait1;
		data[3] = 0;

		data[4] = mask;
		data[5] = ((bits&bit)?(1<<chan_dat):0)|(1<<chan_clk);

		data[6] = wait2;
		data[7] = 0;

		data[8] = mask;
		data[9] = (bits&bit)?(1<<chan_dat):0;

		ret = comedi_do_insnlist(device,&il);

//		printf("comedi_do_insnlist returned %d\n",ret);
	}

}

