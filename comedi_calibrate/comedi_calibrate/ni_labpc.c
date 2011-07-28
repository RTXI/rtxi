/*
   calibration support for NI labpc and compatible boards

   copyright (C) 2003 by Frank Mori Hess

 */

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License as        *
 *   published by                                                          *
 *   the Free Software Foundation; either version 2.1 of the License, or   *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#define _GNU_SOURCE

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <getopt.h>
#include <ctype.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "calib.h"

char ni_labpc_id[] = "$Id: ni_labpc.c,v 1.2 2006-08-24 13:54:43 fmhess Exp $";

int ni_labpc_setup( calibration_setup_t *setup , const char *device_name );
static int cal_ni_labpc( calibration_setup_t *setup );

enum labpc_caldacs
{
	DAC0_GAIN = 1,
	ADC_OFFSET_FINE = 2,
	DAC1_GAIN = 5,
	ADC_GAIN = 6,
	ADC_OFFSET_COARSE_ALT = 7,
	DAC1_OFFSET = 9,
	ADC_POSTGAIN_OFFSET = 10,
	DAC0_OFFSET_ALT = 11,
	ADC_OFFSET_COARSE = 12,
	DAC0_OFFSET = 13,
};
static inline unsigned int DAC_OFFSET( unsigned int channel )
{
	if( channel ) return DAC1_OFFSET;
	else return DAC0_OFFSET;
}
static inline unsigned int DAC_GAIN( unsigned int channel )
{
	if( channel ) return DAC1_GAIN;
	else return DAC0_GAIN;
}

int ni_labpc_setup( calibration_setup_t *setup , const char *device_name )
{
	if( setup->caldac_subdev < 0 )
	{
		fprintf( stderr, "no caldac subdevice found\n");
		return -1;
	}
	setup->status = STATUS_DONE;
	setup->do_cal = cal_ni_labpc;
	setup_caldacs( setup, setup->caldac_subdev );

	return 0;
}


/* load analog input caldacs from eeprom values (depend on range used) */
static void labpc_grab_ai_calibration( calibration_setup_t *setup, unsigned int range )
{
	comedi_calibration_setting_t *current_cal;
	/* points to (end of) analog input bipolar calibration values */
	const int ai_bip_frame = read_eeprom( setup, 127 );
	enum offset_indices
	{
		coarse_offset_index = 0,
		fine_offset_index = -1,
	};
	/* points to (end of) analog input unipolar calibration values */
	const int ai_unip_frame = read_eeprom( setup, 126 );
	/* points to (end of) analog input bipolar calibration values */
	const int bip_gain_frame = read_eeprom( setup, 123 );
	/* points to (end of) analog input bipolar calibration values */
	const int unip_gain_frame = read_eeprom( setup, 122 );
	/* points to (end of) analog input bipolar calibration values */
	const int bip_offset_frame = read_eeprom( setup, 121 );
	/* points to (end of) analog input bipolar calibration values */
	const int unip_offset_frame = read_eeprom( setup, 120 );
	int ai_frame, gain_frame, offset_frame;
	/* eeprom offsets by range */
	int range_to_index[ 14 ] =
	{
		0,
		-2,
		-3,
		-4,
		-5,
		-6,
		-7,
		0,
		-2,
		-3,
		-4,
		-5,
		-6,
		-7,
	};
	int value;

	if( is_unipolar( setup->dev, setup->ad_subdev, 0, range ) )
	{
		ai_frame = ai_unip_frame;
		gain_frame = unip_gain_frame;
		offset_frame = unip_offset_frame;
	}else
	{
		ai_frame = ai_bip_frame;
		gain_frame = bip_gain_frame;
		offset_frame = bip_offset_frame;
	}
	assert( ai_frame >=0 && gain_frame >= 0 && offset_frame >= 0 );

	current_cal = sc_alloc_calibration_setting(setup->new_calibration);

	/* load coarse offset */
	value = read_eeprom( setup, ai_frame + coarse_offset_index );
	assert( value >= 0 );
	update_caldac( setup, ADC_OFFSET_COARSE, value );
	sc_push_caldac( current_cal, setup->caldacs[ ADC_OFFSET_COARSE ] );
	update_caldac( setup, ADC_OFFSET_COARSE_ALT, value );
	sc_push_caldac( current_cal, setup->caldacs[ ADC_OFFSET_COARSE_ALT ] );

	/* load fine offset */
	value = read_eeprom( setup, ai_frame + fine_offset_index );
	assert( value >= 0 );
	update_caldac( setup, ADC_OFFSET_FINE, value );
	sc_push_caldac( current_cal, setup->caldacs[ ADC_OFFSET_FINE ] );

	/* load postgain offset */
	value = read_eeprom( setup, offset_frame + range_to_index[ range ] );
	assert( value >= 0 );
	update_caldac( setup, ADC_POSTGAIN_OFFSET, value );
	sc_push_caldac( current_cal, setup->caldacs[ ADC_POSTGAIN_OFFSET ] );

	/* load gain */
	value = read_eeprom( setup, gain_frame + range_to_index[ range ] );
	assert( value >= 0 );
	update_caldac( setup, ADC_GAIN, value );
	sc_push_caldac( current_cal, setup->caldacs[ ADC_GAIN ] );

	current_cal->subdevice = setup->ad_subdev;
	sc_push_channel( current_cal, SC_ALL_CHANNELS );
	sc_push_range( current_cal, range );
	sc_push_aref( current_cal, SC_ALL_AREFS );

	DPRINT( 0, "loaded adc range %i calibration from eeprom\n", range );
}

static int ao_offset_index( unsigned int channel )
{
	if( channel ) return -2;
	else return 0;
}

static int ao_gain_index( unsigned int channel )
{
	if( channel ) return -3;
	else return -1;
}

/* load analog output caldacs from eeprom values (depend on range used) */
static void labpc_grab_ao_calibration( calibration_setup_t *setup,
	unsigned int channel, unsigned int range)
{
	comedi_calibration_setting_t *current_cal;
	/* points to (end of) analog output bipolar calibration values */
	int ao_bip_frame = read_eeprom( setup, 125 );
	/* points to (end of) analog output bipolar calibration values */
	int ao_unip_frame = read_eeprom( setup, 124 );
	int ao_frame;
	int value;

	current_cal = sc_alloc_calibration_setting(setup->new_calibration);

	if( is_unipolar( setup->dev, setup->da_subdev, 0, range ) )
		ao_frame = ao_unip_frame;
	else
		ao_frame = ao_bip_frame;
	assert( ao_frame >= 0 );

	/* load offset */
	value = read_eeprom( setup, ao_frame + ao_offset_index( channel ) );
	assert( value >= 0 );
	update_caldac( setup, DAC_OFFSET( channel ), value );
	sc_push_caldac( current_cal, setup->caldacs[ DAC_OFFSET( channel ) ] );
	if( channel == 0 )
	{
		update_caldac( setup, DAC0_OFFSET_ALT, value );
		sc_push_caldac( current_cal, setup->caldacs[ DAC0_OFFSET_ALT ] );
	}

	// load gain calibration
	value = read_eeprom( setup, ao_frame + ao_gain_index( channel ) );
	assert( value >= 0 );
	update_caldac( setup, DAC_GAIN( channel ), value );
	sc_push_caldac( current_cal, setup->caldacs[ DAC_GAIN( channel ) ] );

	current_cal->subdevice = setup->da_subdev;
	sc_push_channel( current_cal, channel );
	sc_push_range( current_cal, range );
	sc_push_aref( current_cal, SC_ALL_AREFS );

	DPRINT( 0, "loaded dac channel %i range %i calibration from eeprom\n",
		channel, range );
}


static int cal_ni_labpc( calibration_setup_t *setup )
{
	int range, channel, num_ai_ranges, num_ao_ranges;

	if( comedi_get_version_code( setup->dev ) <= COMEDI_VERSION_CODE( 0, 7, 66 ) )
	{
		DPRINT(0, "WARNING: you need comedi driver version 0.7.67 or later\n"
		 "for this calibration to work properly\n" );
	}

	num_ai_ranges = comedi_get_n_ranges( setup->dev, setup->ad_subdev, 0 );
	assert( num_ai_ranges > 0 );
	num_ao_ranges = comedi_get_n_ranges( setup->dev, setup->da_subdev, 0 );
	assert( num_ao_ranges > 0 );

	for( range = 0; range < num_ai_ranges; range++ )
		labpc_grab_ai_calibration( setup, range );

	if( setup->da_subdev >= 0 && setup->do_output )
	{
		for( channel = 0; channel < 2; channel++ )
		{
			for( range = 0; range < num_ao_ranges; range++ )
				labpc_grab_ao_calibration( setup, channel, range );
		}
	}

	return write_calibration_file(setup->cal_save_file_path, setup->new_calibration);
}

