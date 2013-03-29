/***************************************************************************
	cb.c  -  calibration support for some Measurement computing boards.
                           -------------------

    begin                : Sat Apr 27 2002
    copyright            : (C) 2002,2003 by Frank Mori Hess
    email                : fmhess@users.sourceforge.net

 ***************************************************************************/

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
#include <stdint.h>
#include <assert.h>

#include "calib.h"


char cb_id[] = "$Id: cb.c,v 1.1.1.1 2006-02-05 20:53:20 fmhess Exp $";

struct board_struct{
	char *name;
	int status;
	int (*setup)( calibration_setup_t *setup );
};

static int setup_cb_pci_1xxx( calibration_setup_t *setup );
static int setup_cb_pci_1602_16( calibration_setup_t *setup );

static int cal_cb_pci_1xxx( calibration_setup_t *setup );
static int cal_cb_pci_1602_16( calibration_setup_t *setup );

static int init_observables_1xxx( calibration_setup_t *setup );

static struct board_struct boards[]={
	{ "pci-das1000",	STATUS_DONE,	setup_cb_pci_1xxx },
	{ "pci-das1001",	STATUS_GUESS,	setup_cb_pci_1xxx },
	{ "pci-das1002",	STATUS_GUESS,	setup_cb_pci_1xxx },
	{ "pci-das1200",	STATUS_DONE,	setup_cb_pci_1xxx },
	{ "pci-das1200/jr",	STATUS_GUESS,	setup_cb_pci_1xxx },
	{ "pci-das1602/12",	STATUS_GUESS,	setup_cb_pci_1xxx },
	{ "pci-das1602/16",	STATUS_DONE,	setup_cb_pci_1602_16 },
	{ "pci-das1602/16/jr",	STATUS_GUESS,	setup_cb_pci_1602_16 },
};

static const int num_boards = ( sizeof(boards) / sizeof(boards[0]) );

enum calibration_source_1xxx
{
	CS_1XXX_GROUND = 0,
	CS_1XXX_7V = 1,
	CS_1002_3500mV = 2,
	CS_1002_1750mV = 3,
	CS_1001_88600uV = 3,
	CS_1XXX_875mV = 4,
	CS_1XXX_8600uV = 5,
	CS_1602_16_minus_10V = 5,
	CS_1XXX_DAC0 = 6,
	CS_1XXX_DAC1 = 7,
};
static inline int CS_1XXX_DAC( unsigned int channel )
{
	if( channel )
		return CS_1XXX_DAC1;
	else
		return CS_1XXX_DAC0;
}

int cb_setup( calibration_setup_t *setup, const char *device_name )
{
	unsigned int i;

	for( i = 0; i < num_boards; i++ )
	{
		if( !strcmp( device_name, boards[i].name ) )
		{
			setup->status = boards[i].status;
			return boards[i].setup( setup );
			break;
		}
	}
	if( i == num_boards ) return -1;

	return 0;
}

static int setup_cb_pci_1xxx( calibration_setup_t *setup )
{
	int retval;
	static const int caldac_subdev = 4;
	static const int calpot_subdev = 5;

	if( comedi_get_version_code( setup->dev ) <= COMEDI_VERSION_CODE( 0, 7, 66 ) )
	{
		DPRINT(0, "WARNING: you need comedi driver version 0.7.67 or later\n"
		 "for this calibration to work properly\n" );
	}
	//this larger delay is definitely needed by pci-das1000, possibly not for pci-das1200
	setup->sv_settling_time_ns = 10000000;
	retval = init_observables_1xxx( setup );
	if( retval < 0 ) return retval;
	setup_caldacs( setup, caldac_subdev );
	setup_caldacs( setup, calpot_subdev );
	setup->do_cal = cal_cb_pci_1xxx;
	return 0;
}

static int setup_cb_pci_1602_16( calibration_setup_t *setup )
{
	int retval;
	static const int caldac_subdev = 4;
	static const int calpot_subdev = 5;
	static const int dac08_subdev = 6;

	if( comedi_get_version_code( setup->dev ) <= COMEDI_VERSION_CODE( 0, 7, 66 ) )
	{
		DPRINT(0, "WARNING: you need comedi driver version 0.7.67 or later\n"
		 "for this calibration to work properly\n" );
	}
	setup->sv_settling_time_ns = 10000000;
	setup->sv_order = 12;
	retval = init_observables_1xxx( setup );

	if( retval < 0 ) return retval;
	setup_caldacs( setup, caldac_subdev );
	setup_caldacs( setup, calpot_subdev );
	setup_caldacs( setup, dac08_subdev );
	setup->do_cal = cal_cb_pci_1602_16;
	return 0;
}

static int ai_ground_observable_1xxx( const calibration_setup_t *setup,
	unsigned int channel, unsigned int range )
{
	return 2 * range;
}

static int ai_high_observable_1xxx( const calibration_setup_t *setup,
	unsigned int channel, unsigned int range )
{
	return ai_ground_observable_1xxx( setup, channel, range ) + 1;
}

static int ao_ground_observable_1xxx( const calibration_setup_t *setup,
	unsigned int channel, unsigned int range )
{
	int num_ai_ranges, num_ao_ranges;

	num_ai_ranges = comedi_get_n_ranges( setup->dev, setup->ad_subdev, 0 );
	assert( num_ai_ranges > 0 );
	num_ao_ranges = comedi_get_n_ranges( setup->dev, setup->da_subdev, 0 );
	assert( num_ao_ranges > 0 );

	return 2 * num_ai_ranges + 2 * num_ao_ranges * channel + 2 * range;
}

static int ao_high_observable_1xxx( const calibration_setup_t *setup,
	unsigned int channel, unsigned int range )
{
	return ao_ground_observable_1xxx( setup, channel, range ) + 1;
}

static double ai_low_target_1xxx( calibration_setup_t *setup,
	unsigned int range )
{
	if( is_bipolar( setup->dev, setup->ad_subdev, 0, range ) )
		return 0.0;
	else
		return very_low_target( setup->dev, setup->ad_subdev, 0, range );
}

static int source_eeprom_addr_1xxx( calibration_setup_t *setup, unsigned int range_index )
{
	enum source_eeprom_addr
	{
		EEPROM_7V_CHAN = 0x80,
		EEPROM_3500mV_CHAN = 0x84,
		EEPROM_1750mV_CHAN = 0x88,
		EEPROM_88600uV_CHAN_1001 = 0x88,
		EEPROM_875mV_CHAN = 0x8c,
		EEPROM_8600uV_CHAN = 0x90,
	};
	comedi_range *range;

	range = comedi_get_range( setup->dev, setup->ad_subdev, 0, range_index );
	if( range == NULL ) return -1;

	if( range->max > 7.0 )
		return EEPROM_7V_CHAN;
	else if( range->max > 3.5 )
		return EEPROM_3500mV_CHAN;
	else if( range->max > 1.750 )
		return EEPROM_1750mV_CHAN;
	else if( range->max > 0.875 )
		return EEPROM_875mV_CHAN;
	else if( range->max > .0886 )
		return EEPROM_88600uV_CHAN_1001;
	else if( range->max > 0.0086 )
		return EEPROM_8600uV_CHAN;

	return -1;
}

static int ai_high_cal_source_1xxx( calibration_setup_t *setup, unsigned int range_index )
{
	comedi_range *range;

	range = comedi_get_range( setup->dev, setup->ad_subdev, 0, range_index );
	if( range == NULL ) return -1;

	if( range->max > 7.0 )
		return CS_1XXX_7V;
	else if( range->max > 3.5 )
		return CS_1002_3500mV;
	else if( range->max > 1.750 )
		return CS_1002_1750mV;
	else if( range->max > 0.875 )
		return CS_1XXX_875mV;
	else if( range->max > .0886 )
		return CS_1001_88600uV;
	else if( range->max > 0.0086 )
		return CS_1XXX_8600uV;

	return -1;
}

static int ao_set_high_target_1xxx( calibration_setup_t *setup, unsigned int obs,
	unsigned int range_index )
{
	double target;
	comedi_range *range;

	range = comedi_get_range( setup->dev, setup->da_subdev, 0, range_index );
	if( range == NULL ) return -1;

	target = range->max * 0.9;
	set_target( setup, obs, target );
	return 0;
}

static int init_observables_1xxx( calibration_setup_t *setup )
{
	comedi_insn tmpl, po_tmpl;
	observable *o;
	int retval, range, num_ai_ranges, num_ao_ranges,
		channel, num_channels;
	float target;
	int ai_for_ao_range;

	setup->n_observables = 0;

	memset( &tmpl, 0, sizeof(tmpl) );
	tmpl.insn = INSN_READ;
	tmpl.n = 1;
	tmpl.subdev = setup->ad_subdev;

	num_ai_ranges = comedi_get_n_ranges( setup->dev, setup->ad_subdev, 0 );
	if( num_ai_ranges < 0 ) return -1;

	for( range = 0; range < num_ai_ranges; range++ )
	{
		o = setup->observables + ai_ground_observable_1xxx( setup, 0, range );
		o->reference_source = CS_1XXX_GROUND;
		assert( o->name == NULL );
		asprintf( &o->name, "calibration source %i, range %i, ground referenced",
			o->reference_source, range );
		o->observe_insn = tmpl;
		o->observe_insn.chanspec = CR_PACK( 0, range, AREF_GROUND) |
			CR_ALT_SOURCE | CR_ALT_FILTER;
		o->target = ai_low_target_1xxx( setup, range );
		setup->n_observables++;

		o = setup->observables + ai_high_observable_1xxx( setup, 0, range );;
		retval = ai_high_cal_source_1xxx( setup, range );
		if( retval < 0 ) return -1;
		o->reference_source = retval;
		assert( o->name == NULL );
		asprintf( &o->name, "calibration source %i, range %i, ground referenced",
			o->reference_source, range );
		o->observe_insn = tmpl;
		o->observe_insn.chanspec = CR_PACK( 0, range, AREF_GROUND) |
			CR_ALT_SOURCE | CR_ALT_FILTER;
		retval = cb_actual_source_voltage( setup->dev, setup->eeprom_subdev,
			source_eeprom_addr_1xxx( setup, range ), &target );
		if( retval < 0 ) return -1;
		o->target = target;
		setup->n_observables++;
	}

	if( setup->da_subdev >= 0 )
	{
		num_channels = comedi_get_n_channels( setup->dev, setup->da_subdev );
		if( num_channels < 0 ) return -1;

		num_ao_ranges = comedi_get_n_ranges( setup->dev, setup->da_subdev, 0 );
		if( num_ao_ranges < 0 ) return -1;

		memset( &po_tmpl, 0, sizeof(po_tmpl) );
		po_tmpl.insn = INSN_WRITE;
		po_tmpl.n = 1;
		po_tmpl.subdev = setup->da_subdev;

		ai_for_ao_range = get_bipolar_lowgain( setup->dev, setup->ad_subdev );
		if( ai_for_ao_range < 0 ) return -1;

		for( range = 0; range < num_ao_ranges; range++ )
		{
			for( channel = 0; channel < num_channels; channel++ )
			{
				o = setup->observables + ao_ground_observable_1xxx( setup, channel, range );
				o->reference_source = CS_1XXX_DAC( channel );
				assert( o->name == NULL );
				asprintf( &o->name, "DAC ground calibration source, ch %i, range %i",
					channel, range );
				o->preobserve_insn = po_tmpl;
				o->preobserve_insn.chanspec = CR_PACK( channel, range, AREF_GROUND );
				o->preobserve_insn.data = o->preobserve_data;
				o->observe_insn = tmpl;
				o->observe_insn.chanspec = CR_PACK( 0, ai_for_ao_range, AREF_GROUND) |
					CR_ALT_SOURCE | CR_ALT_FILTER;
				set_target( setup, ao_ground_observable_1xxx( setup, channel, range ), 0.0 );
				setup->n_observables++;

				o = setup->observables + ao_high_observable_1xxx( setup, channel, range );
				o->reference_source = CS_1XXX_DAC( channel );
				assert( o->name == NULL );
				asprintf( &o->name, "DAC high calibration source, ch %i, range %i", channel,
					range );
				o->preobserve_insn = po_tmpl;
				o->preobserve_insn.chanspec = CR_PACK( channel , range, AREF_GROUND );
				o->preobserve_insn.data = o->preobserve_data;
				o->observe_insn = tmpl;
				o->observe_insn.chanspec = CR_PACK( 0, ai_for_ao_range, AREF_GROUND) |
					CR_ALT_SOURCE | CR_ALT_FILTER;
				ao_set_high_target_1xxx( setup, ao_high_observable_1xxx( setup, channel, range ),
					range );
				setup->n_observables++;
			}
		}
	}

	return 0;
}

enum cal_knobs_1xxx
{
	DAC0_GAIN_FINE_1XXX = 0,
	DAC0_GAIN_COARSE_1XXX = 1,
	DAC0_OFFSET_1XXX = 2,
	DAC1_OFFSET_1XXX = 3,
	DAC1_GAIN_FINE_1XXX = 4,
	DAC1_GAIN_COARSE_1XXX = 5,
	ADC_OFFSET_COARSE_1XXX = 6,
	ADC_OFFSET_FINE_1XXX = 7,
	ADC_GAIN_1XXX = 8,
};
static int adc_offset_coarse_1xxx( unsigned int channel )
{
	return ADC_OFFSET_COARSE_1XXX;
}
static int adc_offset_fine_1xxx( unsigned int channel )
{
	return ADC_OFFSET_FINE_1XXX;
}
static int adc_gain_1xxx( unsigned int channel )
{
	return ADC_GAIN_1XXX;
}
static int dac_offset_1xxx( unsigned int channel )
{
	if( channel )
		return DAC1_OFFSET_1XXX;
	else
		return DAC0_OFFSET_1XXX;
}
static int dac_gain_fine_1xxx( unsigned int channel )
{
	if( channel )
		return DAC1_GAIN_FINE_1XXX;
	else
		return DAC0_GAIN_FINE_1XXX;
}
static int dac_gain_coarse_1xxx( unsigned int channel )
{
	if( channel )
		return DAC1_GAIN_COARSE_1XXX;
	else
		return DAC0_GAIN_COARSE_1XXX;
}

static int cal_cb_pci_1xxx( calibration_setup_t *setup )
{
	generic_layout_t layout;

	init_generic_layout( &layout );
	layout.adc_gain = adc_gain_1xxx;
	layout.adc_offset = adc_offset_coarse_1xxx;
	layout.adc_offset_fine = adc_offset_fine_1xxx;
	layout.dac_gain = dac_gain_coarse_1xxx;
	layout.dac_gain_fine = dac_gain_fine_1xxx;
	layout.dac_offset = dac_offset_1xxx;
	layout.adc_high_observable = ai_high_observable_1xxx;
	layout.adc_ground_observable = ai_ground_observable_1xxx;
	layout.dac_high_observable = ao_high_observable_1xxx;
	layout.dac_ground_observable = ao_ground_observable_1xxx;
	layout.adc_fractional_tolerance = get_tolerance( setup, setup->ad_subdev, 1.0 );
	layout.dac_fractional_tolerance = get_tolerance( setup, setup->da_subdev, 1.0 );
	return generic_cal_by_range( setup, &layout );
}

enum cal_knobs_1602_16
{
	DAC0_GAIN_FINE_1602_16 = 0,
	DAC0_GAIN_COARSE_1602_16 = 1,
	DAC0_OFFSET_COARSE_1602_16 = 2,
	DAC1_OFFSET_COARSE_1602_16 = 3,
	DAC1_GAIN_FINE_1602_16 = 4,
	DAC1_GAIN_COARSE_1602_16 = 5,
	DAC0_OFFSET_FINE_1602_16 = 6,
	DAC1_OFFSET_FINE_1602_16 = 7,
	ADC_GAIN_1602_16 = 8,
	ADC_POSTGAIN_OFFSET_1602_16 = 9,
	ADC_PREGAIN_OFFSET_1602_16 = 10,
};
static int dac_gain_coarse_1602_16( unsigned int channel )
{
	if( channel ) return DAC1_GAIN_COARSE_1602_16;
	else return DAC0_GAIN_COARSE_1602_16;
}
static int dac_gain_fine_1602_16( unsigned int channel )
{
	if( channel ) return DAC1_GAIN_FINE_1602_16;
	else return DAC0_GAIN_FINE_1602_16;
}
static int dac_offset_coarse_1602_16( unsigned int channel )
{
	if( channel ) return DAC1_OFFSET_COARSE_1602_16;
	else return DAC0_OFFSET_COARSE_1602_16;
}
static int dac_offset_fine_1602_16( unsigned int channel )
{
	if( channel ) return DAC1_OFFSET_FINE_1602_16;
	else return DAC0_OFFSET_FINE_1602_16;
}
static int adc_gain_1602_16( unsigned int channel )
{
	return ADC_GAIN_1602_16;
}
static int adc_pregain_offset_1602_16( unsigned int channel )
{
	return ADC_PREGAIN_OFFSET_1602_16;
}
static int adc_postgain_offset_1602_16( unsigned int channel )
{
	return ADC_POSTGAIN_OFFSET_1602_16;
}
static int cal_cb_pci_1602_16( calibration_setup_t *setup )
{
	generic_layout_t layout;

	init_generic_layout( &layout );
	layout.adc_gain = adc_gain_1602_16;
	layout.adc_offset = adc_pregain_offset_1602_16;
	layout.adc_postgain_offset = adc_postgain_offset_1602_16;
	layout.dac_gain = dac_gain_coarse_1602_16;
	layout.dac_gain_fine = dac_gain_fine_1602_16;
	layout.dac_offset = dac_offset_coarse_1602_16;
	layout.dac_offset_fine = dac_offset_fine_1602_16;
	layout.adc_high_observable = ai_high_observable_1xxx;
	layout.adc_ground_observable = ai_ground_observable_1xxx;
	layout.dac_high_observable = ao_high_observable_1xxx;
	layout.dac_ground_observable = ao_ground_observable_1xxx;
	layout.adc_fractional_tolerance = get_tolerance( setup, setup->ad_subdev, 1.0 );
	layout.dac_fractional_tolerance = get_tolerance( setup, setup->da_subdev, 1.0 );
	/* The bipolar postgain calibration should be good for both
	 * bipolar and unipolar ranges, so disable separate
	 * unipolar postgain offset calibration (it will fail
	 * horribly anyways if you try it).
	 */
	layout.do_adc_unipolar_postgain = 0;
	return generic_cal_by_range( setup, &layout );
}

// converts calibration source voltages from two 16 bit eeprom values to a floating point value
static float eeprom16_to_source( uint16_t *data )
{
	union translator
	{
		uint32_t bits;
		float	value;
	};

	union translator my_translator;

	my_translator.bits = ( data[ 0 ] & 0xffff ) | ( ( data[ 1 ] << 16 ) & 0xffff0000 );

	return my_translator.value;
}

static float eeprom8_to_source( uint8_t *data )
{
	union translator
	{
		uint32_t bits;
		float	value;
	};
	union translator my_translator;
	int i;

	my_translator.bits = 0;
	for( i = 0; i < 4; i++ )
	{
		my_translator.bits |= ( data[ i ] & 0xffff ) << ( 8 * i );
	}

	return my_translator.value;
}

int cb_actual_source_voltage( comedi_t *dev, unsigned int subdevice, unsigned int eeprom_channel, float *voltage)
{
	int retval;
	unsigned int i;
	lsampl_t data;
	int max_data;

	max_data = comedi_get_maxdata( dev, subdevice, eeprom_channel );

	if( max_data == 0xffff )
	{
		uint16_t word[ 2 ];

		for( i = 0; i < 2; i++ )
		{
			retval = comedi_data_read( dev, subdevice, eeprom_channel + i, 0, 0, &data );
			if( retval < 0 )
			{
				comedi_perror( __FUNCTION__ );
				return retval;
			}
			word[ i ] = data;
		}
		*voltage = eeprom16_to_source( word );
	}else if( max_data == 0xff )
	{
		uint8_t byte[ 4 ];

		for( i = 0; i < 4; i++ )
		{
			retval = comedi_data_read( dev, subdevice, eeprom_channel + i, 0, 0, &data );
			if( retval < 0 )
			{
				comedi_perror( __FUNCTION__ );
				return retval;
			}
			byte[ i ] = data;
		}
		*voltage = eeprom8_to_source( byte );
	}else
	{
		fprintf( stderr, "%s: maxdata = 0x%x invalid for subdevice %i, channel %i\n",
			__FUNCTION__, max_data, subdevice, eeprom_channel);
		return -1;
	}

	DPRINT(1, "eeprom ch 0x%x gives calibration source of %gV\n", eeprom_channel, *voltage);
	return 0;
}
