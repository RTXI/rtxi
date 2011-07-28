/*
    lib/calib.c
    functions for setting calibration

    Copyright (C) 2003 Frank Mori Hess <fmhess@users.sourceforge.net

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation, version 2.1
    of the License.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307
    USA.
*/

#define _GNU_SOURCE

#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "libinternal.h"

static int set_calibration( comedi_t *dev, const comedi_calibration_t *parsed_file,
	unsigned int cal_index );

static int check_cal_file( comedi_t *dev, const comedi_calibration_t *parsed_file )
{
	if( strcmp( comedi_get_driver_name( dev ), parsed_file->driver_name ) )
	{
		COMEDILIB_DEBUG( 3, "driver name does not match '%s' from calibration file\n",
			parsed_file->driver_name );
		return -1;
	}

	if( strcmp( comedi_get_board_name( dev ), parsed_file->board_name ) )
	{
		COMEDILIB_DEBUG( 3, "board name does not match '%s' from calibration file\n",
			parsed_file->board_name );
		return -1;
	}

	return 0;
}

static inline int valid_channel( const comedi_calibration_t *parsed_file,
	unsigned int cal_index, unsigned int channel )
{
	int num_channels, i;

	num_channels = parsed_file->settings[ cal_index ].num_channels;
	if( num_channels == 0 ) return 1;
	for( i = 0; i < num_channels; i++ )
	{
		if( parsed_file->settings[ cal_index ].channels[ i ] == channel )
			return 1;
	}

	return 0;
}

static inline int valid_range( const comedi_calibration_t *parsed_file,
	unsigned int cal_index, unsigned int range )
{
	int num_ranges, i;

	num_ranges = parsed_file->settings[ cal_index ].num_ranges;
	if( num_ranges == 0 ) return 1;
	for( i = 0; i < num_ranges; i++ )
	{
		if( parsed_file->settings[ cal_index ].ranges[ i ] == range )
			return 1;
	}

	return 0;
}

static inline int valid_aref( const comedi_calibration_t *parsed_file,
	unsigned int cal_index, unsigned int aref )
{
	int num_arefs, i;

	num_arefs = parsed_file->settings[ cal_index ].num_arefs;
	if( num_arefs == 0 ) return 1;
	for( i = 0; i < num_arefs; i++ )
	{
		if( parsed_file->settings[ cal_index ].arefs[ i ] == aref )
			return 1;
	}

	return 0;
}

static int apply_calibration( comedi_t *dev, const comedi_calibration_t *parsed_file,
	unsigned int subdev, unsigned int channel, unsigned int range, unsigned int aref )
{
	int num_cals, i, retval;
	int found_cal = 0;

	num_cals = parsed_file->num_settings;

	for( i = 0; i < num_cals; i++ )
	{
		if( parsed_file->settings[ i ].subdevice != subdev ) continue;
		if( valid_range( parsed_file, i, range ) == 0 ) continue;
		if( valid_channel( parsed_file, i, channel ) == 0 ) continue;
		if( valid_aref( parsed_file, i, aref ) == 0 ) continue;

		retval = set_calibration( dev, parsed_file, i );
		if( retval < 0 ) return retval;
		found_cal = 1;
	}
	if( found_cal == 0 )
	{
		COMEDILIB_DEBUG( 3, "failed to find matching calibration\n" );
		return -1;
	}

	return 0;
}

static int set_calibration( comedi_t *dev, const comedi_calibration_t *parsed_file,
	unsigned int cal_index )
{
	int i, retval, num_caldacs;

	num_caldacs = parsed_file->settings[ cal_index ].num_caldacs;
	COMEDILIB_DEBUG( 4, "num_caldacs %i\n", num_caldacs );

	for( i = 0; i < num_caldacs; i++ )
	{
		comedi_caldac_t caldac;

		caldac = parsed_file->settings[ cal_index ].caldacs[ i ];
		COMEDILIB_DEBUG( 4, "subdev %i, ch %i, val %i\n", caldac.subdevice,
			caldac.channel,caldac.value);
		retval = comedi_data_write( dev, caldac.subdevice, caldac.channel,
			0, 0, caldac.value );
		if( retval < 0 ) return retval;
	}

	return 0;
}

EXPORT_ALIAS_DEFAULT(_comedi_apply_parsed_calibration,comedi_apply_parsed_calibration,0.7.20);
int _comedi_apply_parsed_calibration( comedi_t *dev, unsigned int subdev, unsigned int channel,
	unsigned int range, unsigned int aref, const comedi_calibration_t *calibration )
{
	int retval;

	if(!valid_dev(dev)) return -1;
	retval = check_cal_file( dev, calibration );
	if( retval < 0 ) return retval;

	retval = apply_calibration( dev, calibration, subdev, channel, range, aref );
	return retval;
}

/* munge characters in board name that will cause problems with file paths */
static void fixup_board_name( char *name )
{
	while( ( name = strchr( name, '/' ) ) )
	{
		if( name )
		{
			*name = '-';
			name++;
		}
	}
}

EXPORT_ALIAS_DEFAULT(_comedi_get_default_calibration_path,comedi_get_default_calibration_path,0.7.20);
char* _comedi_get_default_calibration_path( comedi_t *dev )
{
	struct stat file_stats;
	char *file_path;
	const char *temp;
	char *board_name;
	const char *driver_name;
	int err;

	if(!valid_dev(dev)) return NULL;
	if( fstat( comedi_fileno( dev ), &file_stats ) < 0 )
	{
		COMEDILIB_DEBUG( 3, "failed to get file stats of comedi device file\n" );
		return NULL;
	}

	driver_name = comedi_get_driver_name( dev );
	if( driver_name == NULL )
	{
		return NULL;
	}
	temp = comedi_get_board_name( dev );
	if( temp == NULL )
	{
		return NULL;
	}
	board_name = strdup( temp );

	fixup_board_name( board_name );
	err = asprintf( &file_path, LOCALSTATEDIR "/lib/comedi/calibrations/%s_%s_comedi%li",
		driver_name, board_name, ( unsigned long ) minor( file_stats.st_rdev ) );

	free( board_name );
	if( err < 0 )
	{
		return NULL;
	}
	return file_path;
}

EXPORT_ALIAS_DEFAULT(_comedi_apply_calibration,comedi_apply_calibration,0.7.20);
int _comedi_apply_calibration( comedi_t *dev, unsigned int subdev, unsigned int channel,
	unsigned int range, unsigned int aref, const char *cal_file_path )
{
	char file_path[ 1024 ];
	int retval;
	comedi_calibration_t *parsed_file;

	if( cal_file_path )
	{
		strncpy( file_path, cal_file_path, sizeof( file_path ) );
	}else
	{
		char *temp;

		temp = comedi_get_default_calibration_path( dev );
		if( temp == NULL ) return -1;
		strncpy( file_path, temp, sizeof( file_path ) );
		free( temp );
	}

	parsed_file = comedi_parse_calibration_file( file_path );
	if( parsed_file == NULL )
	{
		COMEDILIB_DEBUG( 3, "failed to parse calibration file\n" );
		return -1;
	}

	retval = comedi_apply_parsed_calibration( dev, subdev, channel, range, aref, parsed_file );

	comedi_cleanup_calibration( parsed_file );

	return retval;
}

EXPORT_ALIAS_DEFAULT(_comedi_get_hardcal_converter, comedi_get_hardcal_converter, 0.8.0);
int _comedi_get_hardcal_converter(
	comedi_t *dev, unsigned subdevice, unsigned channel, unsigned range,
	enum comedi_conversion_direction direction,
	comedi_polynomial_t* polynomial)
{
	comedi_range *range_ptr = comedi_get_range(dev, subdevice, channel, range);
	lsampl_t maxdata;

	if(range_ptr == NULL)
	{
		return -1;
	}
	maxdata = comedi_get_maxdata(dev, subdevice, channel);
	if(maxdata == 0)
	{
		return -1;
	}
	polynomial->order = 1;
	switch(direction)
	{
	case COMEDI_TO_PHYSICAL:
		polynomial->expansion_origin = 0.;
		polynomial->coefficients[0] = range_ptr->min;
		polynomial->coefficients[1] = (range_ptr->max - range_ptr->min) / maxdata;
		break;
	case COMEDI_FROM_PHYSICAL:
		polynomial->expansion_origin = range_ptr->min;
		polynomial->coefficients[0] = 0.;
		polynomial->coefficients[1] =  maxdata / (range_ptr->max - range_ptr->min);
		break;
	}
	return 0;
}

EXPORT_ALIAS_DEFAULT(_comedi_get_softcal_converter, comedi_get_softcal_converter, 0.8.0);
int _comedi_get_softcal_converter(
	unsigned subdevice, unsigned channel, unsigned range,
	enum comedi_conversion_direction direction,
	const comedi_calibration_t *calibration, comedi_polynomial_t* polynomial)
{
	unsigned i;

	for(i = 0; i < calibration->num_settings; ++i)
	{
		if(calibration->settings[i].subdevice != subdevice) continue;
		if(valid_channel(calibration, i, channel) == 0) continue;
		if(valid_range(calibration, i, range) == 0) continue;
		switch(direction)
		{
		case COMEDI_TO_PHYSICAL:
			if(calibration->settings[i].soft_calibration.to_phys == NULL)
			{
				continue;
			}
			*polynomial = *calibration->settings[i].soft_calibration.to_phys;
			break;
		case COMEDI_FROM_PHYSICAL:
			if(calibration->settings[i].soft_calibration.from_phys == NULL)
			{
				continue;
			}
			*polynomial = *calibration->settings[i].soft_calibration.from_phys;
			break;
		}
		return 0;
	}
	return -1;
}

static double apply_polynomial(const comedi_polynomial_t *polynomial, double input)
{
	double value = 0.;
	double term = 1.;
	unsigned i;
	assert(polynomial->order < COMEDI_MAX_NUM_POLYNOMIAL_COEFFICIENTS);
	for(i = 0; i <= polynomial->order; ++i)
	{
		value += polynomial->coefficients[i] * term;
		term *= input - polynomial->expansion_origin;
	}
	return value;
}

EXPORT_ALIAS_DEFAULT(_comedi_to_physical, comedi_to_physical, 0.8.0);
double _comedi_to_physical(lsampl_t data,
	const comedi_polynomial_t *conversion_polynomial)
{
	return apply_polynomial(conversion_polynomial, data);
}

EXPORT_ALIAS_DEFAULT(_comedi_from_physical, comedi_from_physical, 0.8.0);
lsampl_t _comedi_from_physical(double data,
	const comedi_polynomial_t *conversion_polynomial)
{
	return nearbyint(apply_polynomial(conversion_polynomial, data));
}
