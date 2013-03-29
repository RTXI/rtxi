/***************************************************************************
	cal_common.c  -  shared calibration routines
                             -------------------

    begin                : Fri May 2, 2003
    copyright            : (C) 2003 by Frank Mori Hess
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

#include "calib.h"
#include <assert.h>
#include <stdlib.h>
#include <math.h>

void generic_do_cal( calibration_setup_t *setup,
	comedi_calibration_setting_t *saved_cal, int observable, int caldac )
{
	if( caldac < 0 || observable < 0 ) return;

	cal_binary( setup, observable, caldac );
	sc_push_caldac( saved_cal, setup->caldacs[ caldac ] );
}

void generic_do_relative( calibration_setup_t *setup,
	comedi_calibration_setting_t *saved_cal, int observable1, int observable2, int caldac )
{
	if( caldac < 0 || observable1 < 0 || observable2 < 0 ) return;

	cal_relative_binary( setup, observable1, observable2, caldac );
	sc_push_caldac( saved_cal, setup->caldacs[ caldac ] );
}

void generic_do_linearity( calibration_setup_t *setup,
	comedi_calibration_setting_t *saved_cal, int observable1, int observable2,
	int observable3, int caldac )
{
	if( caldac < 0 || observable1 < 0 || observable2 < 0 || observable3 < 0 )
		return;

	cal_linearity_binary( setup, observable1, observable2, observable3, caldac );
	sc_push_caldac( saved_cal, setup->caldacs[ caldac ] );
}

void generic_peg( calibration_setup_t *setup, int observable, int caldac,
	int maximize )
{
	if( caldac < 0 || observable < 0 ) return;
	peg_binary( setup, observable, caldac, maximize );
}

void generic_prep_adc_caldacs( calibration_setup_t *setup,
	const generic_layout_t *layout, unsigned int channel, unsigned int range )
{
	int retval;

	if( setup->ad_subdev < 0 ) return;

	if( setup->old_calibration == NULL )
	{
		reset_caldac( setup, layout->adc_offset( channel ) );
		reset_caldac( setup, layout->adc_gain( channel ) );
		reset_caldac( setup, layout->adc_offset_fine( channel ) );
		reset_caldac( setup, layout->adc_gain_fine( channel ) );
		reset_caldac( setup, layout->adc_postgain_offset( channel ) );
	}else
	{
		retval = comedi_apply_parsed_calibration( setup->dev, setup->ad_subdev,
			channel, range, AREF_GROUND, setup->old_calibration );
		if( retval < 0 )
		{
			DPRINT( 0, "Failed to apply existing calibration, reseting dac caldacs.\n" );
			reset_caldac( setup, layout->adc_offset( channel ) );
			reset_caldac( setup, layout->adc_gain( channel ) );
			reset_caldac( setup, layout->adc_offset_fine( channel ) );
			reset_caldac( setup, layout->adc_gain_fine( channel ) );
			reset_caldac( setup, layout->adc_postgain_offset( channel ) );
		}
	}
}

void generic_prep_dac_caldacs( calibration_setup_t *setup,
	const generic_layout_t *layout, unsigned int channel, unsigned int range )
{
	int retval;

	if( setup->da_subdev < 0 ) return;

	if( setup->old_calibration == NULL )
	{
		reset_caldac( setup, layout->dac_offset( channel ) );
		reset_caldac( setup, layout->dac_gain( channel ) );
		reset_caldac( setup, layout->dac_linearity( channel ) );
		reset_caldac( setup, layout->dac_offset_fine( channel ) );
		reset_caldac( setup, layout->dac_gain_fine( channel ) );
		reset_caldac( setup, layout->dac_linearity_fine( channel ) );
	}else
	{
		retval = comedi_apply_parsed_calibration( setup->dev, setup->da_subdev,
			channel, range, AREF_GROUND, setup->old_calibration );
		if( retval < 0 )
		{
			DPRINT( 0, "Failed to apply existing calibration, reseting dac caldacs.\n" );
			reset_caldac( setup, layout->dac_offset( channel ) );
			reset_caldac( setup, layout->dac_gain( channel ) );
			reset_caldac( setup, layout->dac_linearity( channel ) );
			reset_caldac( setup, layout->dac_offset_fine( channel ) );
			reset_caldac( setup, layout->dac_gain_fine( channel ) );
			reset_caldac( setup, layout->dac_linearity_fine( channel ) );
		}
	}
}

static void generic_prep_adc_for_dac( calibration_setup_t *setup,
	comedi_calibration_t *calibration, int observable )
{
	unsigned int adc_channel, adc_range;
	int chanspec;

	if( observable < 0 ) return;

	chanspec = setup->observables[ observable ].observe_insn.chanspec;
	adc_channel = CR_CHAN( chanspec );
	adc_range = CR_RANGE( chanspec );

	comedi_apply_parsed_calibration( setup->dev, setup->ad_subdev,
		adc_channel, adc_range, 0, calibration );
}

static int dac_cal_is_good( calibration_setup_t *setup, const generic_layout_t *layout,
	unsigned int channel, unsigned int range )
{
	if( fabs( fractional_offset( setup, setup->da_subdev, channel, range,
		layout->dac_ground_observable( setup, channel, range ) ) ) > layout->dac_fractional_tolerance )
		return 0;
	else if( fabs( fractional_offset( setup, setup->da_subdev, channel, range,
		layout->dac_high_observable( setup, channel, range ) ) ) > layout->dac_fractional_tolerance )
		return 0;

	return 1;
}

static void generic_do_dac_channel( calibration_setup_t *setup, const generic_layout_t *layout ,
	comedi_calibration_t *calibration, comedi_calibration_setting_t *current_cal,
	unsigned int channel, unsigned int range )
{
	static const int max_iterations = 4;
	int i;

	current_cal->subdevice = setup->da_subdev;

	generic_prep_adc_for_dac( setup, calibration,
		layout->dac_ground_observable( setup, channel, range ) );

	for( i = 0; i < max_iterations; i++ )
	{
		generic_do_linearity(setup, current_cal, layout->dac_ground_observable( setup, channel, range ),
			layout->dac_mid_observable( setup, channel, range ),
			layout->dac_high_observable( setup, channel, range ),
			layout->dac_linearity(channel));
		generic_do_relative( setup, current_cal, layout->dac_high_observable( setup, channel, range ),
			layout->dac_ground_observable( setup, channel, range ),layout->dac_gain( channel ) );
		generic_do_cal( setup, current_cal, layout->dac_ground_observable( setup, channel, range ),
			layout->dac_offset( channel ) );

		generic_do_linearity(setup, current_cal, layout->dac_ground_observable( setup, channel, range ),
			layout->dac_mid_observable( setup, channel, range ),
			layout->dac_high_observable( setup, channel, range ),
			layout->dac_linearity_fine(channel));
		generic_do_relative( setup, current_cal, layout->dac_high_observable( setup, channel, range ),
			layout->dac_ground_observable( setup, channel, range ), layout->dac_gain_fine( channel ) );
		generic_do_cal( setup, current_cal, layout->dac_ground_observable( setup, channel, range ),
			layout->dac_offset_fine( channel ) );
		if( dac_cal_is_good( setup, layout, channel, range ) ) break;
	}
	if( i == max_iterations )
		DPRINT(0, "WARNING: unable to calibrate dac channel %i, range %i to desired %g tolerance\n",
			channel, range, layout->dac_fractional_tolerance );
	sc_push_channel( current_cal, channel );
	sc_push_range( current_cal, range );
	sc_push_aref( current_cal, SC_ALL_AREFS );
}

static int adc_cal_is_good( calibration_setup_t *setup, const generic_layout_t *layout,
	unsigned int channel, unsigned int range )
{
	if( fabs( fractional_offset( setup, setup->ad_subdev, channel, range,
		layout->adc_ground_observable( setup, channel, range ) ) ) > layout->adc_fractional_tolerance )
		return 0;
	else if( fabs( fractional_offset( setup, setup->ad_subdev, channel, range,
		layout->adc_high_observable( setup, channel, range ) ) ) > layout->adc_fractional_tolerance )
		return 0;

	return 1;
}

static void generic_do_adc_channel( calibration_setup_t *setup, const generic_layout_t *layout,
	comedi_calibration_setting_t *current_cal, unsigned int channel, unsigned int range )
{
	static const int max_iterations = 4;
	int i;

	current_cal->subdevice = setup->ad_subdev;

	for( i = 0; i < max_iterations; i++ )
	{
		generic_do_relative( setup, current_cal, layout->adc_high_observable( setup, channel, range ),
			layout->adc_ground_observable( setup, channel, range ), layout->adc_gain( channel ) );
		generic_do_cal( setup, current_cal, layout->adc_ground_observable( setup, channel, range ),
			layout->adc_offset( channel ) );
		generic_do_relative( setup, current_cal, layout->adc_high_observable( setup, channel, range ),
			layout->adc_ground_observable( setup, channel, range ), layout->adc_gain_fine( channel ) );
		generic_do_cal( setup, current_cal, layout->adc_ground_observable( setup, channel, range ),
			layout->adc_offset_fine( channel ) );
		if( adc_cal_is_good( setup, layout, channel, range ) ) break;
	}
	if( i == max_iterations )
		DPRINT(0, "WARNING: unable to calibrate adc channel %i, range %i to desired %g tolerance\n",
			channel, range, layout->adc_fractional_tolerance );
	sc_push_channel( current_cal, channel );
	sc_push_range( current_cal, range );
	sc_push_aref( current_cal, SC_ALL_AREFS );
}

static void generic_do_adc_postgain_offset( calibration_setup_t *setup, const generic_layout_t *layout,
	comedi_calibration_setting_t *current_cal, unsigned int channel, int unipolar )
{
	int lowgain, highgain;
	int bip_lowgain;

	current_cal->subdevice = setup->ad_subdev;

	bip_lowgain = get_bipolar_lowgain( setup->dev, setup->ad_subdev );
	if( unipolar )
	{
		lowgain = get_unipolar_lowgain( setup->dev, setup->ad_subdev );
		highgain = get_unipolar_highgain( setup->dev, setup->ad_subdev );
	}else
	{
		lowgain = bip_lowgain;
		highgain = get_bipolar_highgain( setup->dev, setup->ad_subdev );
	}
	generic_prep_adc_caldacs( setup, layout, channel, highgain );
	if( unipolar )
	{
		/* Need to make sure we aren't stuck on zero for unipolar,
		 * by setting pregain offset to maximum.  Use bipolar lowgain
		 * for pegs to make sure we aren't out-of-range. */
		generic_peg( setup, layout->adc_ground_observable( setup, channel, bip_lowgain ),
			layout->adc_offset( channel ), 1 );
		generic_peg( setup, layout->adc_ground_observable( setup, channel, bip_lowgain ),
			layout->adc_offset_fine( channel ), 1 );
	}
	generic_do_relative( setup, current_cal, layout->adc_ground_observable( setup, channel, lowgain ),
		layout->adc_ground_observable( setup, channel, highgain ), layout->adc_postgain_offset( channel ) );

	sc_push_channel( current_cal, channel );
	sc_push_aref( current_cal, SC_ALL_AREFS );
}

int generic_cal_by_channel_and_range( calibration_setup_t *setup,
	const generic_layout_t *layout  )
{
	int range, channel, num_ai_ranges, num_ai_channels, num_ao_ranges,
		num_ao_channels, retval, num_ai_calibrations;
	comedi_calibration_setting_t *current_cal;

	assert( comedi_range_is_chan_specific( setup->dev, setup->ad_subdev ) == 0 );

	num_ai_ranges = comedi_get_n_ranges( setup->dev, setup->ad_subdev, 0 );
	if( num_ai_ranges < 0 ) return -1;

	num_ai_channels = comedi_get_n_channels( setup->dev, setup->ad_subdev );
	if( num_ai_channels < 0 ) return -1;

	if(setup->da_subdev >= 0 && setup->do_output )
	{
		assert( comedi_range_is_chan_specific( setup->dev, setup->da_subdev ) == 0 );

		num_ao_ranges = comedi_get_n_ranges( setup->dev, setup->da_subdev, 0 );
		if( num_ao_ranges < 0 ) return -1;

		num_ao_channels = comedi_get_n_channels( setup->dev, setup->da_subdev );
		if( num_ao_channels < 0 ) return -1;
	}else
		num_ao_ranges = num_ao_channels = 0;

	num_ai_calibrations = num_ai_ranges * num_ai_channels;

	for( channel = 0; channel < num_ai_channels; channel++ )
	{
		int postgain_bip, postgain_unip;

		if( layout->adc_postgain_offset( 0 ) >= 0 )
		{
			/* bipolar postgain */
			current_cal = sc_alloc_calibration_setting(setup->new_calibration);
			generic_do_adc_postgain_offset( setup, layout, current_cal, channel, 0 );
			for( range = 0; range < num_ai_ranges; range++ )
				if( is_bipolar( setup->dev, setup->ad_subdev, channel, range ) )
					sc_push_range( current_cal, range );
			postgain_bip = setup->caldacs[ layout->adc_postgain_offset( channel ) ].value;
			/* unipolar postgain */
			if( layout->do_adc_unipolar_postgain )
			{
				current_cal = sc_alloc_calibration_setting(setup->new_calibration);
				generic_do_adc_postgain_offset( setup, layout, current_cal, channel, 1 );
			}
			for( range = 0; range < num_ai_ranges; range++ )
				if( is_unipolar( setup->dev, setup->ad_subdev, channel, range ) )
					sc_push_range( current_cal, range );
			postgain_unip = setup->caldacs[ layout->adc_postgain_offset( channel ) ].value;
		}else
			postgain_bip = postgain_unip = -1;

		for( range = 0; range < num_ai_ranges; range++ )
		{
			current_cal = sc_alloc_calibration_setting(setup->new_calibration);
			generic_prep_adc_caldacs( setup, layout, channel, range );
			if( is_unipolar( setup->dev, setup->ad_subdev, channel, range ) )
				update_caldac( setup, layout->adc_postgain_offset( channel ), postgain_unip );
			else
				update_caldac( setup, layout->adc_postgain_offset( channel ), postgain_bip );
			generic_do_adc_channel( setup, layout, current_cal, channel, range );
		}
	}
	for( channel = 0; channel < num_ao_channels; channel++ )
	{
		for( range = 0; range < num_ao_ranges; range++ )
		{
			current_cal = sc_alloc_calibration_setting(setup->new_calibration);
			generic_prep_dac_caldacs( setup, layout, channel, range );
			generic_do_dac_channel( setup, layout, setup->new_calibration,
				current_cal, channel, range );
		}
	}

	retval = write_calibration_file(setup->cal_save_file_path, setup->new_calibration);
	return retval;
}

int generic_cal_by_range( calibration_setup_t *setup,
	const generic_layout_t *layout  )
{
	int channel, range, num_ai_ranges, num_ao_ranges,
		num_ao_channels, retval;
	comedi_calibration_setting_t *current_cal;
	int postgain_bip, postgain_unip;

	assert( comedi_range_is_chan_specific( setup->dev, setup->ad_subdev ) == 0 );

	num_ai_ranges = comedi_get_n_ranges( setup->dev, setup->ad_subdev, 0 );
	if( num_ai_ranges < 0 ) return -1;

	if(setup->da_subdev >= 0 && setup->do_output )
	{
		assert( comedi_range_is_chan_specific( setup->dev, setup->da_subdev ) == 0 );

		num_ao_ranges = comedi_get_n_ranges( setup->dev, setup->da_subdev, 0 );
		if( num_ao_ranges < 0 ) return -1;

		num_ao_channels = comedi_get_n_channels( setup->dev, setup->da_subdev );
		if( num_ao_channels < 0 ) return -1;
	}else
		num_ao_ranges = num_ao_channels = 0;

	if( layout->adc_postgain_offset( 0 ) >= 0 )
	{
		/* bipolar postgain */
		current_cal = sc_alloc_calibration_setting(setup->new_calibration);
		generic_do_adc_postgain_offset( setup, layout, current_cal, 0, 0 );
		sc_push_channel( current_cal, SC_ALL_CHANNELS );
		for( range = 0; range < num_ai_ranges; range++ )
			if( is_bipolar( setup->dev, setup->ad_subdev, 0, range ) )
				sc_push_range( current_cal, range );
		postgain_bip = setup->caldacs[ layout->adc_postgain_offset( 0 ) ].value;
		/* unipolar postgain */
		if( layout->do_adc_unipolar_postgain )
		{
			current_cal = sc_alloc_calibration_setting(setup->new_calibration);
			generic_do_adc_postgain_offset( setup, layout, current_cal, 0, 1 );
			sc_push_channel( current_cal, SC_ALL_CHANNELS );
		}
		for( range = 0; range < num_ai_ranges; range++ )
			if( is_unipolar( setup->dev, setup->ad_subdev, 0, range ) )
				sc_push_range( current_cal, range );
		postgain_unip = setup->caldacs[ layout->adc_postgain_offset( 0 ) ].value;
	}else
		postgain_bip = postgain_unip = -1;

	for( range = 0; range < num_ai_ranges; range++ )
	{
		current_cal = sc_alloc_calibration_setting(setup->new_calibration);
		generic_prep_adc_caldacs( setup, layout, 0, range );
		if( is_unipolar( setup->dev, setup->ad_subdev, 0, range ) )
			update_caldac( setup, layout->adc_postgain_offset( 0 ), postgain_unip );
		else
			update_caldac( setup, layout->adc_postgain_offset( 0 ), postgain_bip );
		generic_do_adc_channel( setup, layout, current_cal, 0, range );
		sc_push_channel( current_cal, SC_ALL_CHANNELS );
	}
	for( channel = 0; channel < num_ao_channels; channel++ )
	{
		for( range = 0; range < num_ao_ranges; range++ )
		{
			current_cal = sc_alloc_calibration_setting(setup->new_calibration);
			generic_prep_dac_caldacs( setup, layout, channel, range );
			generic_do_dac_channel( setup, layout, setup->new_calibration,
				current_cal, channel, range );
		}
	}
	retval = write_calibration_file(setup->cal_save_file_path, setup->new_calibration);
	return retval;
}

int generic_cal_ao(calibration_setup_t *setup,
	const generic_layout_t *layout  )
{
	int channel, range, num_ao_ranges,
		num_ao_channels, retval;
	comedi_calibration_setting_t *current_cal;


	if(setup->da_subdev >= 0 && setup->do_output)
	{
		assert( comedi_range_is_chan_specific( setup->dev, setup->da_subdev ) == 0 );

		num_ao_ranges = comedi_get_n_ranges( setup->dev, setup->da_subdev, 0 );
		if( num_ao_ranges < 0 ) return -1;

		num_ao_channels = comedi_get_n_channels( setup->dev, setup->da_subdev );
		if( num_ao_channels < 0 ) return -1;
	}else
		num_ao_ranges = num_ao_channels = 0;
	for( channel = 0; channel < num_ao_channels; channel++ )
	{
		for( range = 0; range < num_ao_ranges; range++ )
		{
			current_cal = sc_alloc_calibration_setting(setup->new_calibration);
			generic_prep_dac_caldacs( setup, layout, channel, range );
			generic_do_dac_channel( setup, layout, setup->new_calibration,
				current_cal, channel, range );
		}
	}
	retval = write_calibration_file(setup->cal_save_file_path, setup->new_calibration);
	return retval;
}

static int dummy_caldac( unsigned int channel )
{
	return -1;
}
static int dummy_observable( const calibration_setup_t *setup,
	unsigned int channel, unsigned int range )
{
	return -1;
}
void init_generic_layout( generic_layout_t *layout )
{
	layout->adc_offset = dummy_caldac;
	layout->adc_offset_fine = dummy_caldac;
	layout->adc_gain = dummy_caldac;
	layout->adc_gain_fine = dummy_caldac;
	layout->adc_postgain_offset = dummy_caldac;
	layout->dac_offset = dummy_caldac;
	layout->dac_offset_fine = dummy_caldac;
	layout->dac_linearity = dummy_caldac;
	layout->dac_linearity_fine = dummy_caldac;
	layout->dac_gain = dummy_caldac;
	layout->dac_gain_fine = dummy_caldac;
	layout->adc_high_observable = dummy_observable;
	layout->adc_ground_observable = dummy_observable;
	layout->dac_high_observable = dummy_observable;
	layout->dac_mid_observable = dummy_observable;
	layout->dac_ground_observable = dummy_observable;
	layout->adc_fractional_tolerance = INFINITY;
	layout->adc_fractional_tolerance = INFINITY;
	layout->do_adc_unipolar_postgain = 0;
}
