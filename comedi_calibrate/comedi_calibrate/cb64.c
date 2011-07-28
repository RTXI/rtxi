/***************************************************************************
	cb64.c  -  calibration support for some Measurement computing boards.
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


char cb64_id[] = "$Id: cb64.c,v 1.2 2006-08-24 13:54:43 fmhess Exp $";

struct board_struct{
	char *name;
	int status;
	int (*setup)( calibration_setup_t *setup );
};

static int setup_cb_pci_64xx( calibration_setup_t *setup );
static int setup_cb_pci_60xx( calibration_setup_t *setup );
static int setup_cb_pci_603x( calibration_setup_t *setup );
static int setup_cb_pci_6052( calibration_setup_t *setup );
static int setup_cb_pci_4020( calibration_setup_t *setup );
static int setup_cb_pci_unknown( calibration_setup_t *setup );

static int cal_cb_pci_64xx( calibration_setup_t *setup );
static int cal_cb_pci_60xx( calibration_setup_t *setup );
static int cal_cb_pci_6052( calibration_setup_t *setup );
static int cal_cb_pci_4020( calibration_setup_t *setup );

static int init_observables_64xx( calibration_setup_t *setup );
static int init_observables_60xx( calibration_setup_t *setup );
static int init_observables_4020( calibration_setup_t *setup );
static int init_observables_unknown( calibration_setup_t *setup );

static struct board_struct boards[]={
	{ "pci-das6402/16",	STATUS_SOME,	setup_cb_pci_64xx },
	{ "pci-das6402/12",	STATUS_GUESS,	setup_cb_pci_64xx },
	{ "pci-das64/m1/16",	STATUS_GUESS,	setup_cb_pci_64xx },
	{ "pci-das64/m2/16",	STATUS_GUESS,	setup_cb_pci_64xx },
	{ "pci-das64/m3/16",	STATUS_GUESS,	setup_cb_pci_64xx },
	{ "pci-das6014",	STATUS_DONE,	setup_cb_pci_60xx },
	{ "pci-das6023",	STATUS_DONE,	setup_cb_pci_60xx },
	{ "pci-das6025",	STATUS_DONE,	setup_cb_pci_60xx },
	{ "pci-das6030",	STATUS_GUESS,	setup_cb_pci_60xx },
	{ "pci-das6031",	STATUS_GUESS,	setup_cb_pci_603x },
	{ "pci-das6032",	STATUS_GUESS,	setup_cb_pci_603x },
	{ "pci-das6033",	STATUS_GUESS,	setup_cb_pci_603x },
	{ "pci-das6034",	STATUS_DONE,	setup_cb_pci_603x },
	{ "pci-das6035",	STATUS_GUESS,	setup_cb_pci_603x },
	{ "pci-das6036",	STATUS_GUESS,	setup_cb_pci_603x },
	{ "pci-das6040",	STATUS_GUESS,	setup_cb_pci_60xx },
	{ "pci-das6052",	STATUS_GUESS,	setup_cb_pci_6052 },
	{ "pci-das6070",	STATUS_GUESS,	setup_cb_pci_60xx },
	{ "pci-das6071",	STATUS_GUESS,	setup_cb_pci_60xx },
	{ "pci-das4020/12",	STATUS_DONE,	setup_cb_pci_4020 },
};

static const int num_boards = ( sizeof(boards) / sizeof(boards[0]) );

enum calibration_source_60xx
{
	CS_60XX_GROUND = 0,
	CS_60XX_10V = 1,
	CS_60XX_5V = 2,
	CS_60XX_500mV = 3,
	CS_60XX_50mV = 4,
	CS_60XX_UNUSED = 5,	// 0V
	CS_60XX_DAC0 = 6,
	CS_60XX_DAC1 = 7,
};

enum calibration_source_64xx
{
	CS_64XX_GROUND = 0,
	CS_64XX_7V = 1,
	CS_64XX_3500mV = 2,
	CS_64XX_1750mV = 3,
	CS_64XX_875mV = 4,
	CS_64XX_8600uV = 5,
	CS_64XX_DAC0 = 6,
	CS_64XX_DAC1 = 7,
};

enum calibration_source_4020
{
	CS_4020_4375mV = 5,
	CS_4020_625mV = 6,
	CS_4020_GROUND = 7,
};

enum cal_knobs_60xx
{
	DAC0_OFFSET_60XX = 0,
	DAC0_GAIN_60XX = 1,
	DAC1_OFFSET_60XX = 2,
	DAC1_GAIN_60XX = 3,
	ADC_OFFSET_FINE_60XX = 4,
	ADC_OFFSET_COARSE_60XX = 5,
	ADC_GAIN_COARSE_60XX = 6,
	ADC_GAIN_FINE_60XX = 7,
};

enum cal_knobs_64xx
{
	DAC0_GAIN_FINE_64XX = 0,
	DAC0_GAIN_COARSE_64XX = 1,
	DAC0_OFFSET_COARSE_64XX = 2,
	DAC1_OFFSET_COARSE_64XX = 3,
	DAC1_GAIN_FINE_64XX = 4,
	DAC1_GAIN_COARSE_64XX = 5,
	DAC0_OFFSET_FINE_64XX = 6,
	DAC1_OFFSET_FINE_64XX = 7,
	ADC_GAIN_64XX = 8,
	ADC_OFFSET_64XX = 9,
};

enum cal_knobs_6052	// FIXME these values are uneducated guesses
{
	DAC0_GAIN_FINE_6052 = 0,
	DAC0_GAIN_COARSE_6052 = 1,
	DAC0_OFFSET_COARSE_6052 = 2,
	DAC1_OFFSET_COARSE_6052 = 3,
	DAC1_GAIN_FINE_6052 = 4,
	DAC1_GAIN_COARSE_6052 = 5,
	DAC0_OFFSET_FINE_6052 = 6,
	DAC1_OFFSET_FINE_6052 = 7,
	ADC_GAIN_6052 = 8,
	ADC_OFFSET_6052 = 9,
};

int cb64_setup( calibration_setup_t *setup, const char *device_name )
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

static int setup_cb_pci_64xx( calibration_setup_t *setup )
{
	static const int caldac_subdev = 6;
	static const int calpot_subdev = 7;

	init_observables_64xx( setup );
	setup_caldacs( setup, caldac_subdev );
	setup_caldacs( setup, calpot_subdev );
	setup->do_cal = cal_cb_pci_64xx;
	return 0;
}

static int setup_cb_pci_60xx( calibration_setup_t *setup )
{
	static const int caldac_subdev = 6;
	init_observables_60xx( setup );
	setup_caldacs( setup, caldac_subdev );
	setup->do_cal = cal_cb_pci_60xx;
	return 0;
}

static int setup_cb_pci_603x(calibration_setup_t *setup)
{
	int retval = setup_cb_pci_60xx(setup);
	//extra delay not needed by 6023 and 6024, but is needed by 6034
	setup->sv_settling_time_ns = 100000000;
	return retval;
}

static int setup_cb_pci_4020( calibration_setup_t *setup )
{
	static const int caldac_subdev = 6;

	init_observables_4020( setup );
	setup_caldacs( setup, caldac_subdev );
	setup->do_cal = cal_cb_pci_4020;
	return 0;
}

static int setup_cb_pci_unknown( calibration_setup_t *setup )
{
	static const int caldac_subdev = 6;
	init_observables_unknown( setup );
	setup_caldacs( setup, caldac_subdev );
	setup->do_cal = 0;
	return 0;
}

static int ai_ground_observable_index_64xx( const calibration_setup_t *setup,
	unsigned int channel, unsigned int range )
{
	return 2 * range;
}

static int ai_high_observable_index_64xx( const calibration_setup_t *setup,
	unsigned int channel, unsigned int range )
{
	return 2 * range + 1;
}

static int ao_ground_observable_index_64xx( const calibration_setup_t *setup,
	unsigned int channel, unsigned int ao_range )
{
	int num_ai_ranges;

	num_ai_ranges = comedi_get_n_ranges( setup->dev, setup->ad_subdev, 0 );
	assert( num_ai_ranges >= 0 );

	return 2 * num_ai_ranges + 2 * channel + 4 * ao_range;
}

static int ao_high_observable_index_64xx( const calibration_setup_t *setup,
	unsigned int channel, unsigned int ao_range )
{
	int num_ai_ranges;

	num_ai_ranges = comedi_get_n_ranges( setup->dev, setup->ad_subdev, 0 );
	assert( num_ai_ranges >= 0 );

	return 2 * num_ai_ranges + 2 * channel + 4 * ao_range + 1;
}

static int source_eeprom_addr_64xx( calibration_setup_t *setup, unsigned int range_index )
{
	enum source_eeprom_addr
	{
		EEPROM_7V_CHAN = 0x30,
		EEPROM_3500mV_CHAN = 0x32,
		EEPROM_1750mV_CHAN = 0x34,
		EEPROM_875mV_CHAN = 0x36,
		EEPROM_8600uV_CHAN = 0x38,
	};
	comedi_range *range;

	range = comedi_get_range( setup->dev, setup->ad_subdev, 0, range_index );
	if( range == NULL )
	{
		fprintf(stderr, "%s: failed to get range\n", __FUNCTION__);
		return -1;
	}
	if( range->max > 7.0 )
		return EEPROM_7V_CHAN;
	else if( range->max > 3.5 )
		return EEPROM_3500mV_CHAN;
	else if( range->max > 0.875 )
		return EEPROM_875mV_CHAN;
	else if( range->max > 0.0086 )
		return EEPROM_8600uV_CHAN;

	return -1;
}

static int ai_low_cal_source_64xx( calibration_setup_t *setup, unsigned int range_index )
{
	comedi_range *range;

	range = comedi_get_range( setup->dev, setup->ad_subdev, 0, range_index );
	if( range == NULL )
	{
		fprintf(stderr, "%s: failed to get range\n", __FUNCTION__);
		return -1;
	}
	if( range->min > -0.001 )
		return CS_64XX_8600uV;
	else
		return CS_64XX_GROUND;
}

static int ai_high_cal_source_64xx( calibration_setup_t *setup, unsigned int range_index )
{
	comedi_range *range;

	range = comedi_get_range( setup->dev, setup->ad_subdev, 0, range_index );
	if( range == NULL )
	{
		fprintf(stderr, "%s: failed to get range\n", __FUNCTION__);
		return -1;
	}
	if( range->max > 7.0 )
		return CS_64XX_7V;
	else if( range->max > 3.5 )
		return CS_64XX_3500mV;
	else if( range->max > 0.875 )
		return CS_64XX_875mV;
	else if( range->max > 0.0086 )
		return CS_64XX_8600uV;

	return -1;
}

static int ao_cal_src_64xx( unsigned int channel )
{
	switch( channel )
	{
	case 0:
		return CS_64XX_DAC0;
		break;
	case 1:
		return CS_64XX_DAC1;
		break;
	default:
		return -1;
		break;
	}
}

static int ao_set_high_target_64xx( calibration_setup_t *setup, unsigned int obs,
	unsigned int range_index )
{
	double target;
	comedi_range *range;

	range = comedi_get_range( setup->dev, setup->da_subdev, 0, range_index );
	if( range == NULL )
	{
		fprintf(stderr, "%s: failed to get range\n", __FUNCTION__);
		return -1;
	}
	target = range->max * 0.9;
	set_target( setup, obs, target );
	return 0;
}

static int init_observables_64xx( calibration_setup_t *setup )
{
	comedi_insn tmpl;
	observable *o;
	int retval;
	float target;
	int range, num_ai_ranges, num_ao_ranges;

	memset( &tmpl, 0, sizeof(tmpl) );
	tmpl.insn = INSN_READ;
	tmpl.n = 1;
	tmpl.subdev = setup->ad_subdev;

	num_ai_ranges = comedi_get_n_ranges( setup->dev, setup->ad_subdev, 0 );
	if( num_ai_ranges < 0 )
	{
		fprintf(stderr, "%s: failed to get number of ai ranges\n", __FUNCTION__);
		return -1;
	}
	setup->n_observables = 0;

	for( range = 0; range < num_ai_ranges; range++ )
	{
		o = setup->observables + ai_ground_observable_index_64xx( setup, 0, range );
		retval = ai_low_cal_source_64xx( setup, range );
		if( retval < 0 ) return -1;
		o->reference_source = retval;
		assert( o->name == NULL );
		asprintf( &o->name, "calibration source %i, range %i, ground referenced",
			o->reference_source, range );
		o->observe_insn = tmpl;
		o->observe_insn.chanspec = CR_PACK( 0, range, AREF_GROUND) | CR_ALT_SOURCE | CR_ALT_FILTER;
		o->target = 0.0;
		setup->n_observables++;

		o = setup->observables + ai_high_observable_index_64xx( setup, 0, range );
		retval = ai_high_cal_source_64xx( setup, range );
		if( retval < 0 ) return -1;
		o->reference_source = retval;
		assert( o->name == NULL );
		asprintf( &o->name, "calibration source %i, range %i, ground referenced",
			o->reference_source, range );
		o->name = "calibration source, 10V bipolar range, ground referenced";
		o->observe_insn = tmpl;
		o->observe_insn.chanspec = CR_PACK( 0, range, AREF_GROUND) | CR_ALT_SOURCE | CR_ALT_FILTER;
		retval = cb_actual_source_voltage( setup->dev, setup->eeprom_subdev,
			source_eeprom_addr_64xx( setup, range ), &target );
		if( retval < 0 ) return -1;
		o->target = target;
		setup->n_observables++;
	}
	if( setup->da_subdev >= 0 )
	{
		comedi_insn po_tmpl;

		memset( &po_tmpl, 0, sizeof(po_tmpl) );
		po_tmpl.insn = INSN_WRITE;
		po_tmpl.n = 1;
		po_tmpl.subdev = setup->da_subdev;

		num_ao_ranges = comedi_get_n_ranges( setup->dev, setup->da_subdev, 0 );
		if( num_ao_ranges < 0 )
		{
			fprintf(stderr, "%s: failed to get number of ao ranges\n", __FUNCTION__);
			return -1;
		}

		for( range = 0; range < num_ao_ranges; range++ )
		{
			int dac_chan, ai_range;
			ai_range = get_bipolar_lowgain( setup->dev, setup->ad_subdev );
			if( ai_range < 0 ) return -1;

			for( dac_chan = 0; dac_chan < 2; dac_chan++ )
			{
				o = setup->observables + ao_ground_observable_index_64xx( setup,
					dac_chan, range );
				o->reference_source = ao_cal_src_64xx( dac_chan );
				assert( o->name == NULL );
				asprintf( &o->name, "dac%i low, range %i, ground referenced", dac_chan, range );
				o->preobserve_insn = po_tmpl;
				o->preobserve_insn.chanspec = CR_PACK( dac_chan, range, AREF_GROUND );
				o->preobserve_insn.data = o->preobserve_data;
				o->observe_insn = tmpl;
				o->observe_insn.chanspec = CR_PACK( 0, ai_range, AREF_GROUND) | CR_ALT_SOURCE | CR_ALT_FILTER;
				set_target( setup, ao_ground_observable_index_64xx( setup, dac_chan, range ), 0.0 );
				setup->n_observables++;

				o = setup->observables + ao_high_observable_index_64xx( setup, dac_chan, range );
				o->reference_source = ao_cal_src_64xx( dac_chan );
				assert( o->name == NULL );
				asprintf( &o->name, "dac%i high, range %i, ground referenced", dac_chan, range );
				o->preobserve_insn = po_tmpl;
				o->preobserve_insn.chanspec = CR_PACK( dac_chan, range, AREF_GROUND );
				o->preobserve_insn.data = o->preobserve_data;
				o->observe_insn = tmpl;
				o->observe_insn.chanspec = CR_PACK( 0, ai_range, AREF_GROUND) | CR_ALT_SOURCE | CR_ALT_FILTER;
				retval = ao_set_high_target_64xx( setup,
					ao_high_observable_index_64xx( setup, dac_chan, range ), range );
				if( retval < 0 ) return -1;
				setup->n_observables++;
			}
		}
	}

	return 0;
}

static int ai_high_observable_index_60xx( const calibration_setup_t *setup,
	unsigned int channel, unsigned int ai_range )
{
	return ai_range * 2 + 1;
}

static int ai_ground_observable_index_60xx( const calibration_setup_t *setup,
	unsigned int channel, unsigned int ai_range )
{
	return ai_range * 2;
}

static int ao_high_observable_index_60xx( const calibration_setup_t *setup,
	unsigned int channel, unsigned int ao_range )
{
	int num_ai_ranges;

	num_ai_ranges = comedi_get_n_ranges( setup->dev, setup->ad_subdev, 0 );
	assert( num_ai_ranges >= 0 );

	return 2 * num_ai_ranges + 2 * channel + 4 * ao_range + 1;
}

static int ao_ground_observable_index_60xx( const calibration_setup_t *setup,
	unsigned int channel, unsigned int ao_range )
{
	int num_ai_ranges;

	num_ai_ranges = comedi_get_n_ranges( setup->dev, setup->ad_subdev, 0 );
	assert( num_ai_ranges >= 0 );

	return 2 * num_ai_ranges + 2 * channel + 4 * ao_range;
}

static int ai_cal_src_voltage_60xx( calibration_setup_t *setup,
	unsigned int calibration_source, float *voltage )
{
	enum source_eeprom_addr
	{
		EEPROM_10V_CHAN = 0x30,
		EEPROM_5V_CHAN = 0x32,
		EEPROM_500mV_CHAN = 0x38,
		EEPROM_50mV_CHAN = 0x3e,
		EEPROM_8mV_CHAN = 0x40,	// bogus?
	};
	int retval;

	switch( calibration_source )
	{
		case CS_60XX_GROUND:
			*voltage = 0.0;
			retval = 0;
			break;
		case CS_60XX_10V:
			retval = cb_actual_source_voltage( setup->dev, setup->eeprom_subdev,
				EEPROM_10V_CHAN, voltage );
			break;
		case CS_60XX_5V:
			retval = cb_actual_source_voltage( setup->dev, setup->eeprom_subdev,
				EEPROM_5V_CHAN, voltage );
			break;
		case CS_60XX_500mV:
			retval = cb_actual_source_voltage( setup->dev, setup->eeprom_subdev,
				EEPROM_500mV_CHAN, voltage );
			break;
		case CS_60XX_50mV:
			retval = cb_actual_source_voltage( setup->dev, setup->eeprom_subdev,
				EEPROM_50mV_CHAN, voltage );
			break;
		default:
			fprintf( stderr, "invalid calibration_source in ai_cal_src_voltage_60xx()\n" );
			retval = -1;
			break;
	}
	return retval;
}

static int high_ai_cal_src_60xx( calibration_setup_t *setup, unsigned int ai_range )
{
	comedi_range *range;

	range = comedi_get_range( setup->dev, setup->ad_subdev, 0, ai_range );
	if( range == NULL )
	{
		fprintf(stderr, "%s: failed to get range\n", __FUNCTION__);
		return -1;
	}
	if( range->max > 9.999 )
		return CS_60XX_10V;
	else if( range->max > 4.999 )
		return CS_60XX_5V;
	else if( range->max > 0.4999 )
		return CS_60XX_500mV;
	else if( range->max > 0.04999 )
		return CS_60XX_50mV;

	return -1;
}

static int ao_cal_src_60xx( unsigned int channel )
{
	switch( channel )
	{
	case 0:
		return CS_60XX_DAC0;
		break;
	case 1:
		return CS_60XX_DAC1;
		break;
	default:
		return -1;
		break;
	}
}

static int init_observables_60xx( calibration_setup_t *setup )
{
	comedi_insn tmpl;
	observable *o;
	int retval, num_ranges, i;
	float target;
	enum
	{
		CAL_SRC_GROUND = 0,
	};

	memset( &tmpl, 0, sizeof(tmpl) );
	tmpl.insn = INSN_READ;
	tmpl.n = 1;
	tmpl.subdev = setup->ad_subdev;

	setup->n_observables = 0;

	num_ranges = comedi_get_n_ranges( setup->dev, setup->ad_subdev, 0 );
	if( num_ranges < 0 )
	{
		fprintf(stderr, "%s: failed to get number of ai ranges\n", __FUNCTION__);
		return -1;
	}
	for( i = 0; i < num_ranges; i++ )
	{
		o = setup->observables + ai_ground_observable_index_60xx( setup, 0, i );
		o->reference_source = CAL_SRC_GROUND;
		assert( o->name == NULL );
		asprintf( &o->name, "calibration source %i, range %i, ground referenced",
			o->reference_source, i );
		o->observe_insn = tmpl;
		o->observe_insn.chanspec = CR_PACK( 0, i, AREF_GROUND) | CR_ALT_SOURCE | CR_ALT_FILTER;
		o->target = 0.0;
		setup->n_observables++;

		o = setup->observables + ai_high_observable_index_60xx( setup, 0, i );
		retval = high_ai_cal_src_60xx( setup, i );
		if( retval < 0 ) return -1;
		o->reference_source = retval;
		assert( o->name == NULL );
		asprintf( &o->name, "calibration source %i, range %i, ground referenced",
			o->reference_source, i );
		o->observe_insn = tmpl;
		o->observe_insn.chanspec = CR_PACK( 0, i, AREF_GROUND) | CR_ALT_SOURCE | CR_ALT_FILTER;
		retval = ai_cal_src_voltage_60xx( setup, o->reference_source, &target );
		if( retval < 0 ) return -1;
		o->target = target;
		setup->n_observables++;
	}

	if( setup->da_subdev >= 0 )
	{
		comedi_insn po_tmpl;

		memset( &po_tmpl, 0, sizeof(po_tmpl) );
		po_tmpl.insn = INSN_WRITE;
		po_tmpl.n = 1;
		po_tmpl.subdev = setup->da_subdev;

		num_ranges = comedi_get_n_ranges( setup->dev, setup->da_subdev, 0 );
		if( num_ranges < 0 )
		{
			fprintf(stderr, "%s: failed to get number of ao ranges\n", __FUNCTION__);
			return -1;
		}

		for( i = 0; i < num_ranges; i++ )
		{
			int dac_chan;
			for( dac_chan = 0; dac_chan < 2; dac_chan++ )
			{
				o = setup->observables + ao_ground_observable_index_60xx( setup,
					dac_chan, i );
				o->reference_source = ao_cal_src_60xx( dac_chan );
				assert( o->name == NULL );
				asprintf( &o->name, "dac%i low, range %i, ground referenced", dac_chan, i );
				o->preobserve_insn = po_tmpl;
				o->preobserve_insn.chanspec = CR_PACK( dac_chan, i, AREF_GROUND );
				o->preobserve_insn.data = o->preobserve_data;
				o->observe_insn = tmpl;
				o->observe_insn.chanspec = CR_PACK( 0, 0, AREF_GROUND) | CR_ALT_SOURCE | CR_ALT_FILTER;
				set_target( setup, ao_ground_observable_index_60xx( setup, dac_chan, i ), 0.0 );
				setup->n_observables++;

				o = setup->observables + ao_high_observable_index_60xx( setup, dac_chan, i );
				o->reference_source = ao_cal_src_60xx( dac_chan );
				assert( o->name == NULL );
				asprintf( &o->name, "dac%i high, range %i, ground referenced", dac_chan, i );
				o->preobserve_insn = po_tmpl;
				o->preobserve_insn.chanspec = CR_PACK( dac_chan, i, AREF_GROUND );
				o->preobserve_insn.data = o->preobserve_data;
				o->observe_insn = tmpl;
				o->observe_insn.chanspec = CR_PACK( 0, 0, AREF_GROUND) | CR_ALT_SOURCE | CR_ALT_FILTER;
				set_target( setup, ao_high_observable_index_60xx( setup, dac_chan, i ), 9.0 );
				setup->n_observables++;
			}
		}
	}

	return 0;
}

static int ai_low_observable_index_4020( const calibration_setup_t *setup,
	unsigned int channel, unsigned int ai_range )
{
	return 4 * channel + 2 * ai_range;
}

static int ai_high_observable_index_4020( const calibration_setup_t *setup,
	unsigned int channel, unsigned int ai_range )
{
	return ai_low_observable_index_4020( setup, channel, ai_range ) + 1;
}

static int high_ai_cal_src_4020( calibration_setup_t *setup, unsigned int ai_range )
{
	comedi_range *range;

	range = comedi_get_range( setup->dev, setup->ad_subdev, 0, ai_range );
	if( range == NULL )
	{
		fprintf(stderr, "%s: failed to get range\n", __FUNCTION__);
		return -1;
	}
	if( range->max > 4.4  )
		return CS_4020_4375mV;
	else if( range->max > 0.7 )
		return CS_4020_625mV;

	return -1;
}

static int source_eeprom_addr_4020( calibration_setup_t *setup, unsigned int range_index )
{
	enum source_eeprom_addr
	{
		EEPROM_4375mV_CHAN = 0x30,
		EEPROM_625mV_CHAN = 0x32,
	};
	comedi_range *range;

	range = comedi_get_range( setup->dev, setup->ad_subdev, 0, range_index );
	if( range == NULL )
	{
		fprintf(stderr, "%s: failed to get range\n", __FUNCTION__);
		return -1;
	}
	if( range->max > 4.4 )
		return EEPROM_4375mV_CHAN;
	else if( range->max > 0.7 )
		return EEPROM_625mV_CHAN;

	return -1;
}

static int init_observables_4020( calibration_setup_t *setup )
{
	comedi_insn tmpl;
	observable *o;
	float target;
	int retval;
	int range, channel, num_ranges, num_channels;

	memset( &tmpl, 0, sizeof(tmpl) );
	tmpl.insn = INSN_READ;
	tmpl.n = 1;
	tmpl.subdev = setup->ad_subdev;

	num_ranges = comedi_get_n_ranges( setup->dev, setup->ad_subdev, 0 );
	if( num_ranges < 0 )
	{
		fprintf(stderr, "%s: failed to get number of ai ranges\n", __FUNCTION__);
		return -1;
	}

	num_channels = comedi_get_n_channels( setup->dev, setup->ad_subdev );
	if( num_channels < 0 )
	{
		fprintf(stderr, "%s: failed to get number of ai channels\n", __FUNCTION__);
		return -1;
	}

	setup->n_observables = 0;

	for( channel = 0; channel < num_channels; channel++ )
	{
		for( range = 0; range < num_ranges; range++ )
		{
			o = setup->observables + ai_low_observable_index_4020( setup, channel, range );
			assert( o->name == NULL );
			asprintf( &o->name, "ground calibration source, ch %i, range %i, ground referenced",
				channel, range );
			o->reference_source = CS_4020_GROUND;
			o->observe_insn = tmpl;
			o->observe_insn.chanspec = CR_PACK( channel, range, AREF_GROUND) | CR_ALT_SOURCE | CR_ALT_FILTER;
			o->target = 0.0;
			setup->n_observables++;

			o = setup->observables + ai_high_observable_index_4020( setup, channel, range );
			retval = high_ai_cal_src_4020( setup, range );
			if( retval < 0 ) return -1;
			o->reference_source = retval;
			assert( o->name == NULL );
			asprintf( &o->name, "calibration source %i, ch %i, range %i, ground referenced",
				o->reference_source, channel, range );
			o->observe_insn = tmpl;
			o->observe_insn.chanspec = CR_PACK( channel, range, AREF_GROUND) | CR_ALT_SOURCE | CR_ALT_FILTER;
			retval = cb_actual_source_voltage( setup->dev, setup->eeprom_subdev,
				source_eeprom_addr_4020( setup, range ), &target );
			if( retval < 0 ) return retval;
			o->target = target;
			setup->n_observables++;
		}
	}

	return 0;
}

static int adc_offset_64xx( unsigned int channel )
{
	return ADC_OFFSET_64XX;
}
static int adc_gain_64xx( unsigned int channel )
{
	return ADC_GAIN_64XX;
}
static int dac_gain_coarse_64xx( unsigned int channel )
{
	if( channel ) return DAC1_GAIN_COARSE_64XX;
	else return DAC0_GAIN_COARSE_64XX;
}
static int dac_gain_fine_64xx( unsigned int channel )
{
	if( channel ) return DAC1_GAIN_FINE_64XX;
	else return DAC0_GAIN_FINE_64XX;
}
static int dac_offset_coarse_64xx( unsigned int channel )
{
	if( channel ) return DAC1_OFFSET_COARSE_64XX;
	else return DAC0_OFFSET_COARSE_64XX;
}
static int dac_offset_fine_64xx( unsigned int channel )
{
	if( channel ) return DAC1_OFFSET_FINE_64XX;
	else return DAC0_OFFSET_FINE_64XX;
}
static int cal_cb_pci_64xx( calibration_setup_t *setup )
{
	generic_layout_t layout;

	init_generic_layout( &layout );
	layout.adc_gain = adc_gain_64xx;
	layout.adc_offset = adc_offset_64xx;
	layout.dac_gain = dac_gain_coarse_64xx;
	layout.dac_gain_fine = dac_gain_fine_64xx;
	layout.dac_offset = dac_offset_coarse_64xx;
	layout.dac_offset_fine = dac_offset_fine_64xx;
	layout.adc_high_observable = ai_high_observable_index_64xx;
	layout.adc_ground_observable = ai_ground_observable_index_64xx;
	layout.dac_high_observable = ao_high_observable_index_64xx;
	layout.dac_ground_observable = ao_ground_observable_index_64xx;
	layout.adc_fractional_tolerance = get_tolerance( setup, setup->ad_subdev, 1.0 );
	layout.dac_fractional_tolerance = get_tolerance( setup, setup->da_subdev, 1.0 );
	return generic_cal_by_range( setup, &layout );
}

static int adc_gain_coarse_60xx( unsigned int channel )
{
	return ADC_GAIN_COARSE_60XX;
}
static int adc_gain_fine_60xx( unsigned int channel )
{
	return ADC_GAIN_FINE_60XX;
}
static int adc_offset_coarse_60xx( unsigned int channel )
{
	return ADC_OFFSET_COARSE_60XX;
}
static int adc_offset_fine_60xx( unsigned int channel )
{
	return ADC_OFFSET_FINE_60XX;
}
static int dac_gain_60xx( unsigned int channel )
{
	if( channel ) return DAC1_GAIN_60XX;
	else return DAC0_GAIN_60XX;
}
static int dac_offset_60xx( unsigned int channel )
{
	if( channel ) return DAC1_OFFSET_60XX;
	else return DAC0_OFFSET_60XX;
}
static int cal_cb_pci_60xx( calibration_setup_t *setup )
{
	generic_layout_t layout;

	init_generic_layout( &layout );
	layout.adc_gain = adc_gain_coarse_60xx;
	layout.adc_gain_fine = adc_gain_fine_60xx;
	layout.adc_offset = adc_offset_coarse_60xx;
	layout.adc_offset_fine = adc_offset_fine_60xx;
	layout.dac_gain = dac_gain_60xx;
	layout.dac_offset = dac_offset_60xx;
	layout.adc_high_observable = ai_high_observable_index_60xx;
	layout.adc_ground_observable = ai_ground_observable_index_60xx;
	layout.dac_high_observable = ao_high_observable_index_60xx;
	layout.dac_ground_observable = ao_ground_observable_index_60xx;
	layout.adc_fractional_tolerance = get_tolerance( setup, setup->ad_subdev, 1.0 );
	layout.dac_fractional_tolerance = get_tolerance( setup, setup->da_subdev, 1.0 );
	return generic_cal_by_range( setup, &layout );
}

static int adc_offset_4020( unsigned int channel )
{
	return channel;
}
static int adc_gain_4020( unsigned int channel )
{
	return 4 + channel;
}

static int cal_cb_pci_4020( calibration_setup_t *setup )
{
	generic_layout_t layout;

	init_generic_layout( &layout );
	layout.adc_offset = adc_offset_4020;
	layout.adc_gain = adc_gain_4020;
	layout.adc_high_observable = ai_high_observable_index_4020;
	layout.adc_ground_observable = ai_low_observable_index_4020;
	layout.adc_fractional_tolerance = get_tolerance( setup, setup->ad_subdev, 1.0 );
	return generic_cal_by_channel_and_range( setup, &layout );
}

static int init_observables_unknown( calibration_setup_t *setup )
{
	comedi_insn tmpl;
	observable *o;
	int retval, num_ranges, i;
	enum
	{
		CAL_SRC_GROUND = 0,
	};
	static const int num_cal_sources = 16;
	static const int range = 0;

	memset( &tmpl, 0, sizeof(tmpl) );
	tmpl.insn = INSN_READ;
	tmpl.n = 1;
	tmpl.subdev = setup->ad_subdev;

	setup->n_observables = 0;

	num_ranges = comedi_get_n_ranges( setup->dev, setup->ad_subdev, 0 );
	if( num_ranges < 0 )
	{
		fprintf(stderr, "%s: failed to get number of ai ranges\n", __FUNCTION__);
		return -1;
	}
	for( i = 0; i < num_cal_sources; i++ )
	{
		o = setup->observables + i;
		o->reference_source = i;
		assert( o->name == NULL );
		asprintf( &o->name, "calibration source %i, range %i, ground referenced",
			o->reference_source, range);
		o->observe_insn = tmpl;
		o->observe_insn.chanspec = CR_PACK( 0, range, AREF_GROUND) | CR_ALT_SOURCE | CR_ALT_FILTER;
		o->target = 0.0;
		setup->n_observables++;
	}
	for(i = 0; i < 16; ++i)
	{
		int eeprom_channel = 0x30 + i;
		float voltage;
		retval = cb_actual_source_voltage(setup->dev, setup->eeprom_subdev, eeprom_channel, &voltage);
		if(retval == 0)
			printf("eeprom ch 0x%x gives calibration source of %gV\n", eeprom_channel, voltage);
		else
			printf("cb_actual_source_voltage returned %i\n", retval);
	}
	return 0;
}

/* We were unable to read the onboard calibration reference voltages on the 6052, so we'll just
 * load caldac values from eeprom until the problem is resolved somehow. */

static int setup_cb_pci_6052( calibration_setup_t *setup )
{
	if( setup->caldac_subdev < 0 )
	{
		fprintf( stderr, "no caldac subdevice found\n");
		return -1;
	}
	setup->do_cal = &cal_cb_pci_6052;
	setup_caldacs(setup, setup->caldac_subdev);

	return 0;
}


/* load analog input caldacs from eeprom values (depend on range used) */
static void grab_ai_calibration_cb_pci_6052(calibration_setup_t *setup, unsigned int range)
{
	comedi_calibration_setting_t *current_cal;
	enum source_eeprom_addr
	{
		EEPROM_5V_CHAN = 0x30,
		EEPROM_3500mV_CHAN = 0x32,
		EEPROM_1750mV_CHAN = 0x34,
		EEPROM_875mV_CHAN = 0x36,
		EEPROM_8600uV_CHAN = 0x38,
	};
/*	According to Measurement Computing the adc eeprom channels are:
ADC Calibration values
50      BIP10V offset
51      BIP10V gain
52      BIP5V offset
53      BIP5V gain
54      BIP2.5V offset
55      BIP2.5V gain
56      BIP1V offset
57      BIP1V gain
58      BIP0.5V offset
59      BIP0.5V gain
5A      BIP0.25V offset
5B      BIP0.25V gain
5C      BIP0.1V offset
5D      BIP0.1V gain
5E      BIP50mV offset
5F      BIP50mV gain
60      UNI10V offset
61      UNI10V gain
62      UNI5V offset
63      UNI5V gain
64      UNI2.5V offset
65      UNI2.5V gain
66      UNI1V offset
67      UNI1V gain
68      UNI0.5V offset
69      UNI0.5V gain
6A      UNI0.25V offset
6B      UNI0.25V gain
6C      UNI0.1V offset
6D      UNI0.1V gain
*/
	int offset_eeprom_channel;
	int gain_eeprom_channel;
	int value;
	const double epsilon = 0.001;
	comedi_range *range_ptr = comedi_get_range(setup->dev, setup->ad_subdev, 0, range);
	if(range_ptr == NULL)
	{
		fprintf(stderr, "%s: comedi_get_range returned NULL\n", __FUNCTION__);
		abort();
	}
	if(is_unipolar(setup->dev, setup->ad_subdev, 0, range) == 0)
	{
		if(fabs(10. - range_ptr->max) < epsilon)
		{
			offset_eeprom_channel = 0x50;
			gain_eeprom_channel = 0x51;
		}else if(fabs(5. - range_ptr->max) < epsilon)
		{
			offset_eeprom_channel = 0x52;
			gain_eeprom_channel = 0x53;
		}else if(fabs(2.5 - range_ptr->max) < epsilon)
		{
			offset_eeprom_channel = 0x54;
			gain_eeprom_channel = 0x55;
		}else if(fabs(1. - range_ptr->max) < epsilon)
		{
			offset_eeprom_channel = 0x56;
			gain_eeprom_channel = 0x57;
		}else if(fabs(0.5 - range_ptr->max) < epsilon)
		{
			offset_eeprom_channel = 0x58;
			gain_eeprom_channel = 0x59;
		}else if(fabs(0.25 - range_ptr->max) < epsilon)
		{
			offset_eeprom_channel = 0x5a;
			gain_eeprom_channel = 0x5b;
		}else if(fabs(0.1 - range_ptr->max) < epsilon)
		{
			offset_eeprom_channel = 0x5c;
			gain_eeprom_channel = 0x5d;
		}else if(fabs(0.5 - range_ptr->max) < epsilon)
		{
			offset_eeprom_channel = 0x5e;
			gain_eeprom_channel = 0x5f;
		}else
		{
			fprintf(stderr, "%s: logic error, failed to determine eeprom channel for range min=%g, max=%g\n",
				__FUNCTION__, range_ptr->min, range_ptr->max);
			abort();
		}
	}else
	{
		if(fabs(10. - range_ptr->max) < epsilon)
		{
			offset_eeprom_channel = 0x60;
			gain_eeprom_channel = 0x61;
		}else if(fabs(5. - range_ptr->max) < epsilon)
		{
			offset_eeprom_channel = 0x62;
			gain_eeprom_channel = 0x63;
		}else if(fabs(2.5 - range_ptr->max) < epsilon)
		{
			offset_eeprom_channel = 0x64;
			gain_eeprom_channel = 0x65;
		}else if(fabs(1. - range_ptr->max) < epsilon)
		{
			offset_eeprom_channel = 0x66;
			gain_eeprom_channel = 0x67;
		}else if(fabs(0.5 - range_ptr->max) < epsilon)
		{
			offset_eeprom_channel = 0x68;
			gain_eeprom_channel = 0x69;
		}else if(fabs(0.25 - range_ptr->max) < epsilon)
		{
			offset_eeprom_channel = 0x6a;
			gain_eeprom_channel = 0x6b;
		}else if(fabs(0.1 - range_ptr->max) < epsilon)
		{
			offset_eeprom_channel = 0x6c;
			gain_eeprom_channel = 0x6d;
		}else
		{
			fprintf(stderr, "%s: logic error, failed to determine eeprom channel for range min=%g, max=%g\n",
				__FUNCTION__, range_ptr->min, range_ptr->max);
			abort();
		}
	}
	current_cal = sc_alloc_calibration_setting(setup->new_calibration);

	/* load offset */
	value = read_eeprom(setup, offset_eeprom_channel);
	if(value < 0)
	{
		fprintf(stderr, "%s: line %i: read_eeprom() returned error\n", __FUNCTION__, __LINE__);
		abort();
	}
	update_caldac(setup, ADC_OFFSET_6052, value);
	sc_push_caldac(current_cal, setup->caldacs[ADC_OFFSET_6052]);

	/* load gain */
	value = read_eeprom(setup, gain_eeprom_channel);
	if(value < 0)
	{
		fprintf(stderr, "%s: line %i: read_eeprom() returned error\n", __FUNCTION__, __LINE__);
		abort();
	}
	update_caldac(setup, ADC_GAIN_6052, value);
	sc_push_caldac(current_cal, setup->caldacs[ADC_GAIN_6052]);

	current_cal->subdevice = setup->ad_subdev;
	sc_push_channel(current_cal, SC_ALL_CHANNELS);
	sc_push_range(current_cal, range);
	sc_push_aref(current_cal, SC_ALL_AREFS);

	DPRINT( 0, "loaded adc range %i calibration from eeprom\n", range );
}

static int dac_gain_coarse_6052( unsigned int channel )
{
	if( channel ) return DAC1_GAIN_COARSE_6052;
	else return DAC0_GAIN_COARSE_6052;
}
static int dac_gain_fine_6052( unsigned int channel )
{
	if( channel ) return DAC1_GAIN_FINE_6052;
	else return DAC0_GAIN_FINE_6052;
}
static int dac_offset_coarse_6052( unsigned int channel )
{
	if( channel ) return DAC1_OFFSET_COARSE_6052;
	else return DAC0_OFFSET_COARSE_6052;
}
static int dac_offset_fine_6052( unsigned int channel )
{
	if( channel ) return DAC1_OFFSET_FINE_6052;
	else return DAC0_OFFSET_FINE_6052;
}

/* load analog output caldacs from eeprom values (depend on range used) */
static void grab_ao_calibration_cb_pci_6052(calibration_setup_t *setup,
	unsigned int channel, unsigned int range)
{
	comedi_calibration_setting_t *current_cal;
	int value;
	int gain_eeprom_chan;
	int offset_eeprom_chan;
/* According to Measurement Computing the dac eeprom channels are:
DAC Calibration values
70      DAC0 BIP10V coarse offset       DAC0 BIP10V fine offset
71      DAC0 BIP10V coarse gain DAC0 BIP10V fine gain
72      DAC0 UNI10V coarse offset       DAC0 UNI10V fine offset
73      DAC0 UNI10V coarse gain DAC0 UNI10V fine gain
74      DAC1 BIP10V coarse offset       DAC1 BIP10V fine offset
75      DAC1 BIP10V coarse gain DAC1 BIP10V fine gain
76      DAC1 UNI10V coarse offset       DAC1 UNI10V fine offset
77      DAC1 UNI10V coarse gain DAC1 UNI10V fine gain
*/
	current_cal = sc_alloc_calibration_setting(setup->new_calibration);

	if(is_unipolar(setup->dev, setup->da_subdev, channel, range))
	{
		switch(channel)
		{
		case 0:
			offset_eeprom_chan = 0x72;
			gain_eeprom_chan = 0x73;
			break;
		case 1:
			offset_eeprom_chan = 0x76;
			gain_eeprom_chan = 0x77;
			break;
		default:
			fprintf(stderr, "%s: line %i: logic error\n", __FUNCTION__, __LINE__);
			abort();
			break;
		}
	}else
	{
		switch(channel)
		{
		case 0:
			offset_eeprom_chan = 0x70;
			gain_eeprom_chan = 0x71;
			break;
		case 1:
			offset_eeprom_chan = 0x74;
			gain_eeprom_chan = 0x75;
			break;
		default:
			fprintf(stderr, "%s: line %i: logic error\n", __FUNCTION__, __LINE__);
			abort();
			break;
		}
	}

	/* load offset */
	value = read_eeprom(setup, offset_eeprom_chan);
	if(value < 0)
	{
		fprintf(stderr, "%s: line %i: read_eeprom() returned error\n", __FUNCTION__, __LINE__);
		abort();
	}
	update_caldac(setup, dac_offset_coarse_6052(channel), (value >> 8) & 0xff);
	sc_push_caldac(current_cal, setup->caldacs[dac_offset_coarse_6052(channel)]);
	update_caldac(setup, dac_offset_fine_6052(channel), value & 0xff);
	sc_push_caldac(current_cal, setup->caldacs[dac_offset_fine_6052(channel)]);

	// load gain calibration
	value = read_eeprom(setup, gain_eeprom_chan);
	if(value < 0)
	{
		fprintf(stderr, "%s: line %i: read_eeprom() returned error\n", __FUNCTION__, __LINE__);
		abort();
	}
	update_caldac(setup, dac_gain_coarse_6052(channel), (value >> 8) & 0xff);
	sc_push_caldac(current_cal, setup->caldacs[dac_gain_coarse_6052(channel)]);
	update_caldac(setup, dac_gain_fine_6052(channel), value & 0xff);
	sc_push_caldac(current_cal, setup->caldacs[dac_gain_fine_6052(channel)]);

	current_cal->subdevice = setup->da_subdev;
	sc_push_channel(current_cal, channel);
	sc_push_range(current_cal, range);
	sc_push_aref(current_cal, SC_ALL_AREFS);

	DPRINT( 0, "loaded dac channel %i range %i calibration from eeprom\n",
		channel, range );
}


static int cal_cb_pci_6052( calibration_setup_t *setup )
{
	int range, channel, num_ai_ranges, num_ao_ranges;

	num_ai_ranges = comedi_get_n_ranges(setup->dev, setup->ad_subdev, 0);
	if(num_ai_ranges < 0)
	{
		fprintf(stderr, "%s: line %i: comedi_get_n_ranges() returned error\n", __FUNCTION__, __LINE__);
		abort();
	}
	num_ao_ranges = comedi_get_n_ranges(setup->dev, setup->da_subdev, 0);
	if(num_ao_ranges < 0)
	{
		fprintf(stderr, "%s: line %i: comedi_get_n_ranges() returned error\n", __FUNCTION__, __LINE__);
		abort();
	}

	for(range = 0; range < num_ai_ranges; range++)
		grab_ai_calibration_cb_pci_6052(setup, range);

	if(setup->da_subdev >= 0 && setup->do_output)
	{
		for(channel = 0; channel < 2; channel++)
		{
			for(range = 0; range < num_ao_ranges; range++)
				grab_ao_calibration_cb_pci_6052(setup, channel, range);
		}
	}

	return write_calibration_file(setup->cal_save_file_path, setup->new_calibration);
}

