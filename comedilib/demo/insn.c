/*
 * Instruction example
 * Part of Comedilib
 *
 * Copyright (c) 1999,2000 David A. Schleef <ds@schleef.org>
 *
 * This file may be freely modified, distributed, and combined with
 * other software, as long as proper attribution is given in the
 * source code.
 */
/*
   This example shows how to use instructions, i.e., comedi_insns.

   Using instructions directly, as in this example, is not recommended
   for the beginner.  Use the higher-level functions such as
   comedi_data_read(), comedi_data_write(), etc., as demonstrated
   in the inp, outp, and dio examples.  Then, if you need the
   additional flexibility that using instructions directly
   provides, study this example and the implementations of
   comedi_data_read(), etc.
 */

#include <stdio.h>
#include <comedilib.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/time.h>
#include <unistd.h>
#include "examples.h"

/*
 * This example does 3 instructions in one system call.  It does
 * a gettimeofday() call, then reads N_SAMPLES samples from an
 * analog input, and the another gettimeofday() call.
 *
 * (Note: The gettimeofday() value is obtained using an INSN_GTOD
 * instruction, which places the seconds value in data[0] and the
 * microseconds in data[1], so the seconds value is limited to
 * 32-bits even on 64-bit systems.)
 */

#define MAX_SAMPLES 128

comedi_t *device;

int main(int argc, char *argv[])
{
	int ret,i;
	comedi_insn insn[3];
	comedi_insnlist il;
	lsampl_t t1[2], t2[2];
	lsampl_t data[MAX_SAMPLES];
	struct parsed_options options;

	init_parsed_options(&options);
	options.n_scan = 10;	/* override default n_scan value to something more suitable */
	parse_options(&options, argc, argv);
	if(options.n_scan > MAX_SAMPLES ){
		fprintf( stderr, "Requested too many samples, reducing to %i\n", MAX_SAMPLES );
		options.n_scan = MAX_SAMPLES;
	}

	device = comedi_open(options.filename);
	if(!device){
		comedi_perror(options.filename);
		exit(-1);
	}

	if(options.verbose){
		printf("measuring device=%s subdevice=%d channel=%d range=%d analog reference=%d\n",
			options.filename, options.subdevice, options.channel, options.range, options.aref);
	}

	/* Set up a the "instruction list", which is just a pointer
	 * to the array of instructions and the number of instructions.
	 */
	il.n_insns=3;
	il.insns=insn;

	/* Instruction 0: perform a gettimeofday() */
	insn[0].insn=INSN_GTOD;
	insn[0].n=2;
	insn[0].data=t1;

	/* Instruction 1: do 10 analog input reads */
	insn[1].insn=INSN_READ;
	insn[1].n = options.n_scan;
	insn[1].data=data;
	insn[1].subdev = options.subdevice;
	insn[1].chanspec=CR_PACK(options.channel, options.range, options.aref);

	/* Instruction 2: perform a gettimeofday() */
	insn[2].insn=INSN_GTOD;
	insn[2].n=2;
	insn[2].data=t2;

	ret=comedi_do_insnlist(device,&il);
	if(ret<0){
		comedi_perror(options.filename);
		exit(-1);
	}

	printf("initial time: %d.%06d\n", t1[0], t1[1]);
	for(i = 0; i < options.n_scan; i++){
		printf("%d\n", data[i]);
	}
	printf("final time: %d.%06d\n", t2[0], t2[1]);

	printf("difference (us): %ld\n",(long)(t2[0]-t1[0]) * 1000000 +
			(t2[1] - t1[1]));

	return 0;
}

