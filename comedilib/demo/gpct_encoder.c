/*
 * NI quadrature encoder example.
 * Part of Comedilib
 *
 * Copyright (c) 2007 Anders Blomdell <anders.blomdell@control.lth.se>
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

int ni_gpct_start_encoder(comedi_t *device, unsigned subdevice,
	unsigned int initial_value,
	int a, int b, int z)
{
	int retval;
	lsampl_t counter_mode;


	retval = comedi_reset(device, subdevice);
	/* set initial counter value by writing to channel 0 */
	retval = comedi_data_write(device, subdevice, 0, 0, 0, initial_value);
	/* set "load a" register to initial_value by writing to channel 1 */
	retval = comedi_data_write(device, subdevice, 1, 0, 0, initial_value);
	/* set "load b" register to initial_value by writing to channel 2 */
	retval = comedi_data_write(device, subdevice, 2, 0, 0, initial_value);

	comedi_set_gate_source(device, subdevice, 0, 0, NI_GPCT_DISABLED_GATE_SELECT);
	comedi_set_gate_source(device, subdevice, 0, 1, NI_GPCT_DISABLED_GATE_SELECT);
	/* note, the comedi_set_other_source calls will fail on 660x boards, since they
	 * don't support user selection of the inputs used for the A/B/Z signals. */
	comedi_set_other_source(device, subdevice, 0, NI_GPCT_SOURCE_ENCODER_A, a);
	comedi_set_other_source(device, subdevice, 0, NI_GPCT_SOURCE_ENCODER_B, b);
	comedi_set_other_source(device, subdevice, 0, NI_GPCT_SOURCE_ENCODER_Z, z);

	counter_mode = (NI_GPCT_COUNTING_MODE_QUADRATURE_X4_BITS |
		NI_GPCT_COUNTING_DIRECTION_HW_UP_DOWN_BITS);
	if (z != NI_GPCT_DISABLED_GATE_SELECT) {
		counter_mode |= (NI_GPCT_INDEX_ENABLE_BIT |
			NI_GPCT_INDEX_PHASE_HIGH_A_HIGH_B_BITS);
	}
	retval = comedi_set_counter_mode(device, subdevice, 0, counter_mode);
	if(retval < 0) return retval;

	retval = comedi_arm(device, subdevice, NI_GPCT_ARM_IMMEDIATE);
	if(retval < 0) return retval;

	return 0;
}

int main(int argc, char *argv[])
{
	comedi_t *device = NULL;
	int subdevice = -1;
	int a = NI_GPCT_DISABLED_OTHER_SELECT;
	int b = NI_GPCT_DISABLED_OTHER_SELECT;
	int z = NI_GPCT_DISABLED_OTHER_SELECT;
	unsigned int initial_value = 0;
	int retval;

	{
		int c;
		while (-1 != (c = getopt(argc, argv, "f:s:A:B:Z:I:"))) {
			switch (c) {
			case 'f':
				device = comedi_open(optarg);
				if(!device) {
					comedi_perror(optarg);
					exit(-1);
				}
				break;
			case 's':
				subdevice = strtoul(optarg, NULL, 0);
				break;
			case 'A':
				/* TODO: Should we pass the value directly, i.e. could anybody
				*       be interested in values besides PFIx/DISABLED */
				a = NI_GPCT_PFI_OTHER_SELECT(strtoul(optarg, NULL, 0));
				break;
			case 'B':
				/* TODO: Should we pass the value directly, i.e. could anybody
				*       be interested in values besides PFIx/DISABLED */
				b = NI_GPCT_PFI_OTHER_SELECT(strtoul(optarg, NULL, 0));
				break;
			case 'Z':
				/* TODO: Should we pass the value directly, i.e. could anybody
				*       be interested in values besides PFIx/DISABLED */
				z = NI_GPCT_PFI_OTHER_SELECT(strtoul(optarg, NULL, 0));
				break;
			case 'I':
				initial_value = strtoul(optarg, NULL, 0);
				break;
			}
		}
	}

	/*FIXME: check that device is counter */
	printf("Initiating encoder on subdevice %d.\n", subdevice);

	retval = ni_gpct_start_encoder(device, subdevice, initial_value, a, b, z);

	if(retval < 0) return retval;

	return 0;
}
