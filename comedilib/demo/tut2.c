/*
 * Tutorial example #2
 * Part of Comedilib
 *
 * Copyright (c) 1999,2000 David A. Schleef <ds@schleef.org>
 * Copyright (c) 2008 Frank Mori Hess <fmhess@users.sourceforge.net>
 *
 * This file may be freely modified, distributed, and combined with
 * other software, as long as proper attribution is given in the
 * source code.
 */

#include <stdio.h>		/* for printf() */
#include <stdlib.h>
#include <comedilib.h>

int subdev = 0;			/* change this to your input subdevice */
int chan = 0;			/* change this to your channel */
int range = 0;			/* more on this later */
int aref = AREF_GROUND;		/* more on this later */
const char filename[] = "/dev/comedi0";

/* figure out if we are talking to a hardware-calibrated or software-calibrated board,
	then obtain a comedi_polynomial_t which can be used with comedi_to_physical */
int get_converter(comedi_t *device, unsigned subdevice, unsigned channel,
	unsigned range, comedi_polynomial_t *converter)
{
	int retval;
	int flags;

	flags = comedi_get_subdevice_flags(device, subdevice);
	if(flags < 0)
	{
		comedi_perror("comedi_get_subdevice_flags");
		return -1;
	}

	if(flags & SDF_SOFT_CALIBRATED) /* board uses software calibration */
	{
		char *calibration_file_path = comedi_get_default_calibration_path(device);

		/* parse a calibration file which was produced by the
			comedi_soft_calibrate program */
		comedi_calibration_t* parsed_calibration =
			comedi_parse_calibration_file(calibration_file_path);
		free(calibration_file_path);
		if(parsed_calibration == NULL)
		{
			comedi_perror("comedi_parse_calibration_file");
			return -1;
		}

		/* get the comedi_polynomial_t for the subdevice/channel/range
			we are interested in */
		retval = comedi_get_softcal_converter(subdevice, channel, range,
			COMEDI_TO_PHYSICAL, parsed_calibration, converter);
		comedi_cleanup_calibration(parsed_calibration);
		if(retval < 0)
		{
			comedi_perror("comedi_get_softcal_converter");
			return -1;
		}
	}else /* board uses hardware calibration */
	{
		retval = comedi_get_hardcal_converter(device, subdevice, channel, range,
			COMEDI_TO_PHYSICAL, converter);
		if(retval < 0)
		{
			comedi_perror("comedi_get_hardcal_converter");
			return -1;
		}
	}

	return 0;
}

int main(int argc, char *argv[])
{
	comedi_t *it;
	lsampl_t data;
	double physical_value;
	int retval;
	comedi_polynomial_t converter;

	it = comedi_open(filename);
	if(it == NULL)
	{
		comedi_perror(filename);
		return -1;
	}

	retval = comedi_data_read(it, subdev, chan, range, aref, &data);
	if(retval < 0)
	{
		comedi_perror(filename);
		return -1;
	}

	retval = get_converter(it, subdev, chan, range, &converter);
	if(retval < 0)
	{
		return -1;
	}

	physical_value = comedi_to_physical(data, &converter);
	printf("%d %g\n", data, physical_value);

	return 0;
}
