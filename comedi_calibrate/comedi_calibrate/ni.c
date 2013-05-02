/*
   A little auto-calibration utility, for boards
   that support it.

   copyright (C) 1999,2000,2001,2002 by David Schleef
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


char ni_id[] = "$Id: ni.c,v 1.8 2009-09-22 14:55:18 fmhess Exp $";

struct board_struct{
	char *name;
	int status;
	int (*cal)( calibration_setup_t *setup);
	void (*setup_observables)( calibration_setup_t *setup );
	int ref_eeprom_lsb;
	int ref_eeprom_msb;
};

static int ni_setup_board( calibration_setup_t *setup , const char *device_name );
static void ni_setup_observables( calibration_setup_t *setup );
static void ni_setup_observables_611x( calibration_setup_t *setup );
static void ni67xx_setup_observables( calibration_setup_t *setup );

static int cal_ni_at_mio_16de_10(calibration_setup_t *setup);
static int cal_ni_at_mio_16e_2(calibration_setup_t *setup);
static int cal_ni_at_mio_64e_3(calibration_setup_t *setup);
static int cal_ni_daqcard_ai_16xe_50(calibration_setup_t *setup);
static int cal_ni_at_mio_16e_1(calibration_setup_t *setup);
static int cal_ni_pci_mio_16e_1(calibration_setup_t *setup);
static int cal_ni_pci_6014(calibration_setup_t *setup);
static int cal_ni_pci_6024e(calibration_setup_t *setup);
static int cal_ni_pci_6032e(calibration_setup_t *setup);
static int cal_ni_pci_6034e(calibration_setup_t *setup);
static int cal_ni_pci_6035e(calibration_setup_t *setup);
static int cal_ni_pci_6036e(calibration_setup_t *setup);
static int cal_ni_pci_6071e(calibration_setup_t *setup);
static int cal_ni_pxi_6071e(calibration_setup_t *setup);
static int cal_ni_at_mio_16e_10(calibration_setup_t *setup);
static int cal_ni_pci_mio_16xe_50(calibration_setup_t *setup);
static int cal_ni_pci_6023e(calibration_setup_t *setup);
static int cal_ni_at_mio_16xe_50(calibration_setup_t *setup);
static int cal_ni_pci_mio_16xe_10(calibration_setup_t *setup);
static int cal_ni_pci_6052e(calibration_setup_t *setup);
static int cal_ni_daqcard_ai_16e_4(calibration_setup_t *setup);
static int cal_ni_pci_611x(calibration_setup_t *setup);
static int cal_ni_pci_mio_16e_4(calibration_setup_t *setup);
static int cal_ni_daqcard_6062e(calibration_setup_t *setup);
static int cal_ni_daqcard_6024e(calibration_setup_t *setup);
static int cal_ni_daqcard_6036e(calibration_setup_t *setup);
static int cal_ni_pci_6711(calibration_setup_t *setup);

static double ni_get_reference( calibration_setup_t *setup, int lsb_loc,int msb_loc);

static struct board_struct boards[]={
	{ "at-ai-16xe-10", STATUS_UNKNOWN, NULL, ni_setup_observables, 0x1b7, 0x1b8 },
	{ "at-mio-16de-10", STATUS_DONE, cal_ni_at_mio_16de_10, ni_setup_observables, 0x1a7, 0x1a8 },
	{ "at-mio-16e-1", STATUS_DONE, cal_ni_at_mio_16e_1, ni_setup_observables, 0x1a9, 0x1aa },
	{ "at-mio-16e-2", STATUS_DONE, cal_ni_at_mio_16e_2, ni_setup_observables, 0x1a9, 0x1aa },
	{ "at-mio-16e-10", STATUS_DONE, cal_ni_at_mio_16e_10, ni_setup_observables, 0x1a7, 0x1a8 },
	{ "at-mio-16xe-10", STATUS_UNKNOWN, NULL, ni_setup_observables, 0x1b7, 0x1b8 },
	{ "at-mio-16xe-50", STATUS_DONE, cal_ni_at_mio_16xe_50, ni_setup_observables, 0x1b5, 0x1b6 },
	{ "at-mio-64e-3", STATUS_SOME, cal_ni_at_mio_64e_3, ni_setup_observables, 0x1a9, 0x1aa},
	{ "DAQCard-ai-16e-4", STATUS_DONE, cal_ni_daqcard_ai_16e_4, ni_setup_observables, 0x1b5, 0x1b6 },
	{ "DAQCard-ai-16xe-50", STATUS_DONE, cal_ni_daqcard_ai_16xe_50, ni_setup_observables, 0x1be, 0x1bf },
	{ "DAQCard-6024E", STATUS_SOME, cal_ni_daqcard_6024e, ni_setup_observables, -1, -1 },
	{ "DAQCard-6036E", STATUS_DONE, cal_ni_daqcard_6036e, ni_setup_observables, 0x1ab, 0x1ac },
	{ "DAQCard-6062E", STATUS_DONE, cal_ni_daqcard_6062e, ni_setup_observables, 0x1a9, 0x1aa },
	{ "pci-mio-16e-1", STATUS_DONE, cal_ni_pci_mio_16e_1, ni_setup_observables, 0x1a9, 0x1aa },
	{ "pci-mio-16e-4", STATUS_DONE, cal_ni_pci_mio_16e_4, ni_setup_observables, 0x1a9, 0x1aa },
	{ "pci-mio-16xe-10", STATUS_DONE,	cal_ni_pci_mio_16xe_10,	ni_setup_observables, 0x1ae, 0x1af },
	{ "pci-mio-16xe-50", STATUS_SOME, cal_ni_pci_mio_16xe_50, ni_setup_observables, 0x1b5, 0x1b6 },
	{ "pci-6014", STATUS_DONE, cal_ni_pci_6014, ni_setup_observables, 0x1ab, 0x1ac },
	{ "pci-6023e", STATUS_DONE, cal_ni_pci_6023e, ni_setup_observables, 0x1bb, 0x1bc },
	{ "pci-6024e", STATUS_DONE, cal_ni_pci_6024e, ni_setup_observables, 0x1af, 0x1b0 },
	{ "pci-6025e", STATUS_DONE, cal_ni_pci_6035e, ni_setup_observables, 0x1af, 0x1b0 },
	{ "pci-6031e", STATUS_DONE, cal_ni_pci_mio_16xe_10, ni_setup_observables, 0x1ae, 0x1af },
	{ "pci-6032e", STATUS_DONE, cal_ni_pci_6032e, ni_setup_observables, 0x1ae, 0x1af },
	{ "pci-6033e", STATUS_DONE, cal_ni_pci_6032e, ni_setup_observables, 0x1b7, 0x1b8 },
	{ "pci-6034e", STATUS_DONE, cal_ni_pci_6034e, ni_setup_observables, 0x1bb, 0x1bc },
	{ "pci-6035e", STATUS_DONE, cal_ni_pci_6035e, ni_setup_observables, 0x1af, 0x1b0 },
	{ "pci-6036e", STATUS_DONE, cal_ni_pci_6036e, ni_setup_observables, 0x1ab, 0x1ac },
	{ "pci-6052e", STATUS_DONE, cal_ni_pci_6052e, ni_setup_observables, 0x19f, 0x1a0 },
	{ "pci-6071e", STATUS_DONE, cal_ni_pci_6071e, ni_setup_observables, 0x1a9, 0x1aa },
	{ "pci-6110", STATUS_DONE, cal_ni_pci_611x, ni_setup_observables_611x, 0x1d4, 0x1d5 },
	{ "pci-6111", STATUS_DONE, cal_ni_pci_611x, ni_setup_observables_611x, 0x1d4, 0x1d5 },
	{ "pxi-6025e", STATUS_GUESS, cal_ni_pci_6035e, ni_setup_observables, -1, -1 },
	{ "pxi-6030e", STATUS_GUESS, cal_ni_pci_mio_16xe_10, ni_setup_observables, -1, -1 },
	{ "pxi-6031e", STATUS_DONE, cal_ni_pci_mio_16xe_10, ni_setup_observables, 0x1ae, 0x1af },
	{ "pxi-6040e", STATUS_GUESS, cal_ni_pci_mio_16e_4, ni_setup_observables, -1, -1 },
	{ "pxi-6052e", STATUS_GUESS, cal_ni_pci_6052e, ni_setup_observables, -1, -1 },
	{ "pxi-6070e", STATUS_UNKNOWN, NULL, ni_setup_observables, -1, -1 },
	{ "pci-6070e", STATUS_UNKNOWN, NULL, ni_setup_observables, -1, -1 },
	{ "pxi-6071e", STATUS_GUESS, cal_ni_pxi_6071e, ni_setup_observables, -1, -1 },
	{ "pci-6711", STATUS_DONE, cal_ni_pci_6711, ni67xx_setup_observables, 0x1d4, 0x1d5},
	{ "pci-6713", STATUS_DONE, cal_ni_pci_6711, ni67xx_setup_observables, 0x1d4, 0x1d5},
	{ "pci-6731", STATUS_GUESS, cal_ni_pci_6711, ni67xx_setup_observables, -1, -1},
	{ "pci-6733", STATUS_GUESS, cal_ni_pci_6711, ni67xx_setup_observables, -1, -1},
	{ "pxi-6711", STATUS_GUESS, cal_ni_pci_6711, ni67xx_setup_observables, -1, -1},
	{ "pxi-6713", STATUS_DONE, cal_ni_pci_6711, ni67xx_setup_observables, 0x1d4, 0x1d5},
	{ "pxi-6731", STATUS_GUESS, cal_ni_pci_6711, ni67xx_setup_observables, -1, -1},
	{ "pxi-6733", STATUS_GUESS, cal_ni_pci_6711, ni67xx_setup_observables, -1, -1},
#if 0
	{ "at-mio-64e-3",	cal_ni_16e_1 },
#endif
};
#define n_boards (sizeof(boards)/sizeof(boards[0]))

static const int ni_num_observables = 20;
enum observables{
	ni_zero_offset_low = 0,
	ni_zero_offset_high,
	ni_reference_low,
	ni_unip_zero_offset_low,
	ni_unip_zero_offset_high,
	ni_unip_reference_low,
	ni_ao0_zero_offset,
	ni_ao0_reference,
	ni_ao0_linearity,
	ni_ao1_zero_offset,
	ni_ao1_reference,
	ni_ao1_linearity,
	ni_ao0_unip_zero_offset,
	ni_ao0_unip_reference,
	ni_ao0_unip_low_linearity,
	ni_ao0_unip_mid_linearity,
	ni_ao1_unip_zero_offset,
	ni_ao1_unip_reference,
	ni_ao1_unip_low_linearity,
	ni_ao1_unip_mid_linearity,
};
static inline unsigned int ni_ao_zero_offset( unsigned int channel )
{
	if( channel ) return ni_ao1_zero_offset;
	else return ni_ao0_zero_offset;
}
static inline unsigned int ni_ao_reference( unsigned int channel )
{
	if( channel ) return ni_ao1_reference;
	else return ni_ao0_reference;
}
static inline unsigned int ni_ao_mid_linearity( unsigned int channel )
{
	if( channel ) return ni_ao1_linearity;
	else return ni_ao0_linearity;
}
static inline unsigned int ni_ao_unip_zero_offset( unsigned int channel )
{
	if( channel ) return ni_ao1_unip_zero_offset;
	else return ni_ao0_unip_zero_offset;
}
static inline unsigned int ni_ao_unip_reference( unsigned int channel )
{
	if( channel ) return ni_ao1_unip_reference;
	else return ni_ao0_unip_reference;
}
static inline unsigned int ni_ao_unip_low_linearity( unsigned int channel )
{
	if( channel ) return ni_ao1_unip_low_linearity;
	else return ni_ao0_unip_low_linearity;
}
static inline unsigned int ni_ao_unip_mid_linearity( unsigned int channel )
{
	if( channel ) return ni_ao1_unip_mid_linearity;
	else return ni_ao0_unip_mid_linearity;
}

static const int num_ao_observables_611x = 4;
static int ni_ao_zero_offset_611x( const calibration_setup_t *setup,
	unsigned int channel, unsigned int range ) {
	assert( range == 0 );
	return 2 * channel;
};
static int ni_ao_reference_611x( const calibration_setup_t *setup,
	unsigned int channel, unsigned int range ) {
	assert( range == 0 );
	return 2 * channel + 1;
};
static int ni_zero_offset_611x( const calibration_setup_t *setup,
	unsigned int channel, unsigned int range ) {
	return num_ao_observables_611x + 8 * range + 2 * channel;
};
static int ni_reference_611x( const calibration_setup_t *setup,
	unsigned int channel, unsigned int range ) {
	return num_ao_observables_611x + 8 * range + 2 * channel + 1;
};

enum reference_sources {
	REF_GND_GND = 0,
	REF_AOGND_AIGND = 1,
	REF_DAC0_GND = 2,
	REF_DAC1_GND = 3,
	REF_CALSRC_CALSRC = 4,
	REF_CALSRC_GND = 5,
	REF_DAC0_CALSRC = 6,
	REF_DAC1_CALSRC = 7,
};
static inline unsigned int REF_DAC_GND( unsigned int channel )
{
	if( channel ) return REF_DAC1_GND;
	else return REF_DAC0_GND;
}
static inline unsigned int REF_DAC_CALSRC( unsigned int channel )
{
	if( channel ) return REF_DAC1_CALSRC;
	else return REF_DAC0_CALSRC;
}

static struct board_struct* ni_board( calibration_setup_t *setup )
{
	return setup->private_data;
}

typedef struct
{
	int adc_pregain_offset;
	int adc_postgain_offset;
	int adc_gain;
	int adc_pregain_offset_fine;
	int adc_postgain_offset_fine;
	int adc_gain_fine;
	int adc_unip_offset;
	int adc_unip_offset_fine;
	int dac_offset[ 2 ];
	int dac_offset_fine[ 2 ];
	int dac_gain[ 2 ];
	int dac_gain_fine[ 2 ];
	int dac_linearity[ 2 ];
} ni_caldac_layout_t;

static int cal_ni_generic( calibration_setup_t *setup,
	const ni_caldac_layout_t *layout );

static inline void init_ni_caldac_layout( ni_caldac_layout_t *layout )
{
	int i;

	layout->adc_pregain_offset = -1;
	layout->adc_postgain_offset = -1;
	layout->adc_gain = -1;
	layout->adc_unip_offset = -1;
	layout->adc_unip_offset_fine = -1;
	layout->adc_pregain_offset_fine = -1;
	layout->adc_postgain_offset_fine = -1;
	layout->adc_gain_fine = -1;
	for( i = 0; i < 2; i++ )
	{
		layout->dac_offset[ i ] = -1;
		layout->dac_offset_fine[ i ] = -1;
		layout->dac_gain[ i ] = -1;
		layout->dac_gain_fine[ i ] = -1;
		layout->dac_linearity[ i ] = -1;
	}
}

int ni_setup( calibration_setup_t *setup , const char *device_name )
{
	int retval;

	retval = ni_setup_board( setup, device_name );
	if( retval < 0 )
	{
		return retval;
	}
	setup_caldacs( setup, setup->caldac_subdev );

	return 0;
}

static int ni_setup_board( calibration_setup_t *setup, const char *device_name )
{
	int i;

	for(i = 0; i < n_boards; i++ ){
		if(!strcmp( device_name, boards[i].name )){
			setup->status = boards[i].status;
			setup->do_cal = boards[i].cal;
			setup->private_data = &boards[ i ];
			boards[i].setup_observables( setup );
			break;
		}
	}
	if( i == n_boards ) return -1;
	return 0;
}

static void ni_setup_ao_observables( calibration_setup_t *setup )
{
	observable *o;
	comedi_insn tmpl, po_tmpl;
	unsigned int channel;
	int ai_bipolar_lowgain;
	int ao_bipolar_lowgain;
	int ao_unipolar_lowgain;

	ai_bipolar_lowgain = get_bipolar_lowgain( setup->dev, setup->ad_subdev);
	assert(ai_bipolar_lowgain >= 0);
	ao_bipolar_lowgain = get_bipolar_lowgain( setup->dev, setup->da_subdev);
	assert(ao_bipolar_lowgain >= 0);
	ao_unipolar_lowgain = get_unipolar_lowgain( setup->dev, setup->da_subdev);

	memset(&tmpl,0,sizeof(tmpl));
	tmpl.insn = INSN_READ;
	tmpl.n = 1;
	tmpl.subdev = setup->ad_subdev;

	memset(&po_tmpl, 0, sizeof(po_tmpl));
	po_tmpl.insn = INSN_WRITE;
	po_tmpl.n = 1;
	po_tmpl.subdev = setup->da_subdev;

	for( channel = 0; channel < 2; channel++ )
	{
		/* ao zero offset */
		o = setup->observables + ni_ao_zero_offset( channel );
		assert( o->name == NULL );
		asprintf( &o->name, "ao %i, zero offset, low gain", channel );
		o->preobserve_insn = po_tmpl;
		o->preobserve_insn.chanspec = CR_PACK(channel,ao_bipolar_lowgain,0);
		o->preobserve_insn.data = o->preobserve_data;
		o->observe_insn = tmpl;
		o->observe_insn.chanspec =
			CR_PACK(REF_DAC_GND( channel ),ai_bipolar_lowgain,AREF_OTHER)
			| CR_ALT_SOURCE | CR_ALT_FILTER;
		o->reference_source = REF_DAC_GND( channel );
		set_target( setup, ni_ao_zero_offset( channel ),0.0);

		/* ao gain */
		o = setup->observables + ni_ao_reference( channel );
		assert( o->name == NULL );
		asprintf( &o->name, "ao %i, reference voltage, low gain", channel );
		o->preobserve_insn = po_tmpl;
		o->preobserve_insn.chanspec = CR_PACK(channel,ao_bipolar_lowgain,0);
		o->preobserve_insn.data = o->preobserve_data;
		o->observe_insn = tmpl;
		o->observe_insn.chanspec =
			CR_PACK(REF_DAC_GND( channel ),ai_bipolar_lowgain,AREF_OTHER)
			| CR_ALT_SOURCE | CR_ALT_FILTER;
		o->reference_source = REF_DAC_GND( channel );
		set_target( setup, ni_ao_reference( channel ),8.0);

		/* ao linearity, mid */
		o = setup->observables + ni_ao_mid_linearity( channel );
		assert( o->name == NULL );
		asprintf( &o->name, "ao %i, linearity (mid), low gain", channel );
		o->preobserve_insn = po_tmpl;
		o->preobserve_insn.chanspec = CR_PACK(channel,ao_bipolar_lowgain,0);
		o->preobserve_insn.data = o->preobserve_data;
		o->observe_insn = tmpl;
		o->observe_insn.chanspec =
			CR_PACK(REF_DAC_GND( channel ),ai_bipolar_lowgain,AREF_OTHER)
			| CR_ALT_SOURCE | CR_ALT_FILTER;
		o->reference_source = REF_DAC_GND( channel );
		set_target( setup, ni_ao_mid_linearity( channel ),4.0);

		if( ao_unipolar_lowgain >= 0 )
		{
			/* ao unipolar zero offset */
			o = setup->observables + ni_ao_unip_zero_offset( channel );
			assert( o->name == NULL );
			asprintf( &o->name, "ao %i, unipolar zero offset, low gain", channel );
			o->preobserve_insn = po_tmpl;
			o->preobserve_insn.chanspec = CR_PACK(channel,ao_unipolar_lowgain,0);
			o->preobserve_insn.data = o->preobserve_data;
			o->observe_insn = tmpl;
			o->observe_insn.chanspec =
				CR_PACK(REF_DAC_GND( channel ),ai_bipolar_lowgain,AREF_OTHER)
				| CR_ALT_SOURCE | CR_ALT_FILTER;
			o->reference_source = REF_DAC_GND( channel );
			set_target( setup, ni_ao_unip_zero_offset( channel ),0.0);

			/* ao unipolar gain */
			o = setup->observables + ni_ao_unip_reference( channel );
			assert( o->name == NULL );
			asprintf( &o->name, "ao %i, unipolar high, low gain", channel );
			o->preobserve_insn = po_tmpl;
			o->preobserve_insn.chanspec = CR_PACK(channel,ao_unipolar_lowgain,0);
			o->preobserve_insn.data = o->preobserve_data;
			o->observe_insn = tmpl;
			o->observe_insn.chanspec =
				CR_PACK(REF_DAC_GND( channel ),ai_bipolar_lowgain,AREF_OTHER)
				| CR_ALT_SOURCE | CR_ALT_FILTER;
			o->reference_source = REF_DAC_GND( channel );
			set_target( setup, ni_ao_unip_reference( channel ), 9.0);

			/* ao unipolar linearity, mid */
			o = setup->observables + ni_ao_unip_mid_linearity( channel );
			assert( o->name == NULL );
			asprintf( &o->name, "ao %i, unipolar linearity (mid), low gain", channel );
			o->preobserve_insn = po_tmpl;
			o->preobserve_insn.chanspec = CR_PACK(channel,ao_unipolar_lowgain,0);
			o->preobserve_insn.data = o->preobserve_data;
			o->observe_insn = tmpl;
			o->observe_insn.chanspec =
				CR_PACK(REF_DAC_GND( channel ),ai_bipolar_lowgain,AREF_OTHER)
				| CR_ALT_SOURCE | CR_ALT_FILTER;
			o->reference_source = REF_DAC_GND( channel );
			set_target( setup, ni_ao_unip_mid_linearity( channel ), 5.0);

			/* ao unipolar linearity, low */
			o = setup->observables + ni_ao_unip_low_linearity( channel );
			assert( o->name == NULL );
			asprintf( &o->name, "ao %i, unipolar linearity (low), low gain", channel );
			o->preobserve_insn = po_tmpl;
			o->preobserve_insn.chanspec = CR_PACK(channel,ao_unipolar_lowgain,0);
			o->preobserve_insn.data = o->preobserve_data;
			o->observe_insn = tmpl;
			o->observe_insn.chanspec =
				CR_PACK(REF_DAC_GND( channel ),ai_bipolar_lowgain,AREF_OTHER)
				| CR_ALT_SOURCE | CR_ALT_FILTER;
			o->reference_source = REF_DAC_GND( channel );
			set_target( setup, ni_ao_unip_low_linearity( channel ), 1.0);
		}
	}
}

static void ni_setup_observables( calibration_setup_t *setup )
{
	comedi_insn tmpl;
	int bipolar_lowgain;
	int bipolar_highgain;
	int unipolar_lowgain;
	int unipolar_highgain;
	double voltage_reference;
	observable *o;

	bipolar_lowgain = get_bipolar_lowgain( setup->dev, setup->ad_subdev);
	bipolar_highgain = get_bipolar_highgain( setup->dev, setup->ad_subdev);
	unipolar_lowgain = get_unipolar_lowgain( setup->dev, setup->ad_subdev);
	unipolar_highgain = get_unipolar_highgain( setup->dev, setup->ad_subdev);

	if( ni_board( setup )->ref_eeprom_lsb >= 0 &&
		ni_board( setup )->ref_eeprom_msb >= 0 )
	{
		voltage_reference = ni_get_reference( setup,
			ni_board( setup )->ref_eeprom_lsb, ni_board( setup )->ref_eeprom_msb );
	}else
	{
		DPRINT( 0, "WARNING: unknown eeprom address for reference voltage\n"
			"correction.  This might be fixable if you send us an eeprom dump\n"
			"(see the demo/eeprom_dump program).\n");
		voltage_reference = 5.0;
	}

	memset(&tmpl,0,sizeof(tmpl));
	tmpl.insn = INSN_READ;
	tmpl.n = 1;
	tmpl.subdev = setup->ad_subdev;

	setup->n_observables = ni_num_observables;

	/* 0 offset, low gain */
	o = setup->observables + ni_zero_offset_low;
	o->name = "ai, bipolar zero offset, low gain";
	o->observe_insn = tmpl;
	o->observe_insn.chanspec = CR_PACK(REF_GND_GND,bipolar_lowgain,AREF_OTHER)
		| CR_ALT_SOURCE | CR_ALT_FILTER;
	o->reference_source = REF_GND_GND;
	o->target = 0;

	/* 0 offset, high gain */
	o = setup->observables + ni_zero_offset_high;
	o->name = "ai, bipolar zero offset, high gain";
	o->observe_insn = tmpl;
	o->observe_insn.chanspec = CR_PACK(REF_GND_GND,bipolar_highgain,AREF_OTHER)
		| CR_ALT_SOURCE | CR_ALT_FILTER;
	o->reference_source = REF_GND_GND;
	o->target = 0;

	/* voltage reference */
	o = setup->observables + ni_reference_low;
	o->name = "ai, bipolar voltage reference, low gain";
	o->observe_insn = tmpl;
	o->observe_insn.chanspec = CR_PACK(REF_CALSRC_GND,bipolar_lowgain,AREF_OTHER)
		| CR_ALT_SOURCE | CR_ALT_FILTER;
	o->reference_source = REF_CALSRC_GND;
	o->target = voltage_reference;

	if(unipolar_lowgain>=0){
		o = setup->observables + ni_unip_zero_offset_low;
		o->name = "ai, unipolar zero offset, low gain";
		o->observe_insn = tmpl;
		o->observe_insn.chanspec =
			CR_PACK(REF_GND_GND,unipolar_lowgain,AREF_OTHER)
			| CR_ALT_SOURCE | CR_ALT_FILTER;
		o->reference_source = REF_GND_GND;
		o->target = very_low_target( setup->dev, setup->ad_subdev, 0, unipolar_lowgain );

		o = setup->observables + ni_unip_reference_low;
		o->name = "ai, unipolar voltage reference, low gain";
		o->observe_insn = tmpl;
		o->observe_insn.chanspec =
			CR_PACK(REF_CALSRC_GND,unipolar_lowgain,AREF_OTHER)
			| CR_ALT_SOURCE | CR_ALT_FILTER;
		o->reference_source = REF_CALSRC_GND;
		o->target = voltage_reference;
	}

	if(unipolar_highgain >= 0)
	{
		o = setup->observables + ni_unip_zero_offset_high;
		o->name = "ai, unipolar zero offset, high gain";
		o->observe_insn = tmpl;
		o->observe_insn.chanspec =
			CR_PACK(REF_GND_GND,unipolar_highgain,AREF_OTHER)
			| CR_ALT_SOURCE | CR_ALT_FILTER;
		o->reference_source = REF_GND_GND;
		o->target = very_low_target( setup->dev, setup->ad_subdev, 0, unipolar_highgain );
	}

	if(setup->da_subdev >= 0)
		ni_setup_ao_observables( setup );
}

/* for +-50V and +-20V ranges, the reference source goes 0V
 * to 50V instead of 0V to 5V */
static unsigned int cal_gain_register_bits_611x( double reference, double *voltage )
{
	int bits;

	bits = 200.0 * ( *voltage / reference );
	if( bits > 200 ) bits = 200;
	if( bits < 0 ) bits = 0;

	*voltage = reference * ( bits / 200.0 );
	return bits;
}

static unsigned int ref_source_611x( unsigned int ref_source, unsigned int cal_gain_bits )
{
	return ( ref_source & 0xf ) | ( ( cal_gain_bits << 4 ) & 0xff0 );
}

static void reference_target_611x( calibration_setup_t *setup,
	observable *o, double master_reference, unsigned int range )
{
	int cal_gain_reg_bits;
	double reference;
	double target;
	comedi_range *range_ptr;

	range_ptr = comedi_get_range( setup->dev, setup->ad_subdev, 0, range );
	assert( range_ptr != NULL );
	if( range_ptr->max > 19.0 ) reference = 10 * master_reference;
	else reference = master_reference;
	target = range_ptr->max * 0.8;

	cal_gain_reg_bits = cal_gain_register_bits_611x( reference, &target );

	o->reference_source = ref_source_611x( REF_CALSRC_GND, cal_gain_reg_bits );
	o->target = target;
}

static void ni_setup_observables_611x( calibration_setup_t *setup )
{
	comedi_insn tmpl;
	comedi_insn po_tmpl;
	int range, channel;
	double master_reference;
	observable *o;
	int num_ai_channels, num_ai_ranges;
	static const int num_ao_channels = 2;

	setup->sv_settling_time_ns = 10000000;
	setup->sv_order = 14;

	master_reference = ni_get_reference( setup,
		ni_board( setup )->ref_eeprom_lsb, ni_board( setup )->ref_eeprom_msb );

	memset(&tmpl,0,sizeof(tmpl));
	tmpl.insn = INSN_READ;
	tmpl.n = 1;
	tmpl.subdev = setup->ad_subdev;

	num_ai_channels = comedi_get_n_channels( setup->dev, setup->ad_subdev );
	assert( num_ai_channels >= 0 );
	num_ai_ranges = comedi_get_n_ranges( setup->dev, setup->ad_subdev, 0 );
	assert( num_ai_ranges >= 0 );

	for( channel = 0; channel < num_ai_channels; channel++ )
	{
		for( range = 0; range < num_ai_ranges; range++ )
		{
			/* 0 offset */
			o = setup->observables + ni_zero_offset_611x( setup, channel, range );
			assert( o->name == NULL );
			asprintf( &o->name, "ai, ch %i, range %i, zero offset",
				channel, range );
			o->observe_insn = tmpl;
			o->observe_insn.chanspec = CR_PACK( channel, range, AREF_DIFF )
				| CR_ALT_SOURCE | CR_ALT_FILTER;
			o->reference_source = REF_GND_GND;
			o->target = 0.0;

			/* voltage reference */
			o = setup->observables + ni_reference_611x( setup, channel, range );
			assert( o->name == NULL );
			asprintf( &o->name, "ai, ch %i, range %i, voltage reference",
				channel, range );
			o->observe_insn = tmpl;
			o->observe_insn.chanspec = CR_PACK( channel, range, AREF_DIFF )
				| CR_ALT_SOURCE | CR_ALT_FILTER;
			reference_target_611x( setup, o, master_reference, range );
		}
	}

	memset(&po_tmpl,0,sizeof(po_tmpl));
	po_tmpl.insn = INSN_WRITE;
	po_tmpl.n = 1;
	po_tmpl.subdev = setup->da_subdev;

	for( channel = 0; channel < num_ao_channels; channel ++ )
	{
		static const int ai_range_for_ao = 2;

		/* ao zero offset */
		o = setup->observables + ni_ao_zero_offset_611x( setup, channel, 0 );
		assert( o->name == NULL );
		asprintf( &o->name, "ao ch %i, zero offset", channel );
		o->preobserve_insn = po_tmpl;
		o->preobserve_insn.chanspec = CR_PACK( channel, 0, AREF_GROUND );
		o->preobserve_insn.data = o->preobserve_data;
		o->observe_insn = tmpl;
		o->observe_insn.chanspec = CR_PACK( 0, ai_range_for_ao, AREF_DIFF )
			| CR_ALT_SOURCE | CR_ALT_FILTER;
		o->reference_source = REF_DAC_GND( channel );
		set_target( setup, ni_ao_zero_offset_611x( setup, channel, 0 ), 0.0 );

		/* ao gain */
		o = setup->observables + ni_ao_reference_611x( setup, channel, 0 );
		assert( o->name == NULL );
		asprintf( &o->name, "ao ch %i, reference voltage", channel );
		o->preobserve_insn = po_tmpl;
		o->preobserve_insn.chanspec = CR_PACK( channel, 0, AREF_GROUND );
		o->preobserve_insn.data = o->preobserve_data;
		o->observe_insn = tmpl;
		o->observe_insn.chanspec = CR_PACK( 0, ai_range_for_ao, AREF_DIFF )
			| CR_ALT_SOURCE | CR_ALT_FILTER;
		o->reference_source = REF_DAC_GND( channel );
		set_target( setup, ni_ao_reference_611x( setup, channel, 0 ), 5.0 );
	}

	setup->n_observables = num_ao_observables_611x + 2 * num_ai_ranges * num_ai_channels;
}

static int cal_ni_daqcard_ai_16xe_50(calibration_setup_t *setup)
{
	ni_caldac_layout_t layout;

	init_ni_caldac_layout( &layout );
	layout.adc_pregain_offset = 8;
	layout.adc_postgain_offset = 2;
	layout.adc_gain = 0;
	layout.adc_gain_fine = 1;

	return cal_ni_generic( setup, &layout );
}

static int cal_ni_at_mio_16xe_50(calibration_setup_t *setup)
{
	ni_caldac_layout_t layout;
	init_ni_caldac_layout( &layout );
	layout.adc_pregain_offset = 8;
	layout.adc_postgain_offset = 2;
	layout.adc_gain = 0;
	layout.adc_gain_fine = 1;
	layout.dac_offset[ 0 ] = 6;
	layout.dac_gain[ 0 ] = 4;
	layout.dac_offset[ 1 ] = 7;
	layout.dac_gain[ 1 ] = 5;

	return cal_ni_generic( setup, &layout );
}

static int cal_ni_pci_mio_16xe_10(calibration_setup_t *setup)
{
	ni_caldac_layout_t layout;

	init_ni_caldac_layout( &layout );
	layout.adc_pregain_offset = 8;
	layout.adc_postgain_offset = 2;
	layout.adc_postgain_offset_fine = 3;
	layout.adc_gain = 0;
	layout.adc_gain_fine = 1;
	layout.dac_offset[ 0 ] = 6;
	layout.dac_gain[ 0 ] = 4;
	layout.dac_offset[ 1 ] = 7;
	layout.dac_gain[ 1 ] = 5;

	return cal_ni_generic( setup, &layout );
}

static int cal_ni_at_mio_16e_1(calibration_setup_t *setup)
{
	ni_caldac_layout_t layout;

	init_ni_caldac_layout( &layout );
	layout.adc_pregain_offset = 0;
	layout.adc_postgain_offset = 1;
	layout.adc_gain = 3;
	layout.adc_unip_offset = 2;
	layout.dac_offset[0] = 5;
	layout.dac_gain[0] = 6;
	layout.dac_linearity[0] = 4;
	layout.dac_offset[1] = 8;
	layout.dac_gain[1] = 9;
	layout.dac_linearity[1] = 7;

	return cal_ni_generic( setup, &layout );
}

static int cal_ni_at_mio_16e_2(calibration_setup_t *setup)
{
	return cal_ni_at_mio_16e_1(setup);
}

static int cal_ni_pci_mio_16e_1(calibration_setup_t *setup)
{
	ni_caldac_layout_t layout;

	init_ni_caldac_layout( &layout );
	layout.adc_pregain_offset = 0;
	layout.adc_postgain_offset = 1;
	layout.adc_unip_offset = 2;
	layout.adc_gain = 3;
	layout.dac_offset[ 0 ] = 5;
	layout.dac_gain[ 0 ] = 6;
	layout.dac_linearity[ 0 ] = 4;
	layout.dac_offset[ 1 ] = 8;
	layout.dac_gain[ 1 ] = 9;
	layout.dac_linearity[ 1 ] = 7;

	return cal_ni_generic( setup, &layout );
}

static int cal_ni_pci_6014(calibration_setup_t *setup)
{
	ni_caldac_layout_t layout;

	init_ni_caldac_layout( &layout );
	layout.adc_pregain_offset = 0;
	layout.adc_postgain_offset = 4;
	layout.adc_pregain_offset_fine = 8;
	layout.adc_gain = 2;
	layout.dac_offset[0] = 6;
	layout.dac_offset_fine[0] = 10;
	layout.dac_gain[0] = 7;
	layout.dac_gain_fine[0] = 11;
	layout.dac_offset[1] = 9;
	layout.dac_offset_fine[1] = 1;
	layout.dac_gain[1] = 3;
	layout.dac_gain_fine[1] = 5;
	return cal_ni_generic( setup, &layout );
}

static int cal_ni_pci_6032e(calibration_setup_t *setup)
{
	ni_caldac_layout_t layout;

	init_ni_caldac_layout( &layout );
	layout.adc_pregain_offset = 8;
	layout.adc_postgain_offset = 2;
	layout.adc_postgain_offset_fine = 3;
	layout.adc_gain = 0;
	layout.adc_gain_fine = 1;

	return cal_ni_generic( setup, &layout );
}

static int cal_ni_pci_6034e(calibration_setup_t *setup)
{
	ni_caldac_layout_t layout;

	init_ni_caldac_layout( &layout );
	layout.adc_pregain_offset = 0;
	layout.adc_pregain_offset_fine = 8;
	layout.adc_postgain_offset = 4;
	layout.adc_gain = 2;

	return cal_ni_generic( setup, &layout );
}

static int cal_ni_at_mio_16de_10(calibration_setup_t *setup)
{
	if(comedi_get_version_code(setup->dev) <= COMEDI_VERSION_CODE(0, 7, 70))
	{
		DPRINT(0, "WARNING: you need comedi driver version 0.7.71 or later\n"
		 "for this calibration to work properly\n" );
	}
	return cal_ni_pci_6035e(setup);
}

static int cal_ni_pci_6035e(calibration_setup_t *setup)
{
	/* this is for the ad8804_debug caldac */
	ni_caldac_layout_t layout;

	init_ni_caldac_layout( &layout );
	layout.adc_pregain_offset = 0;
	layout.adc_pregain_offset_fine = 8;
	layout.adc_postgain_offset = 4;
	layout.adc_gain = 2;
	layout.dac_offset[ 0 ] = 6;
	layout.dac_gain[ 0 ] = 11;
	layout.dac_linearity[ 0 ] = 10;
	layout.dac_offset[ 1 ] = 9;
	layout.dac_gain[ 1 ] = 5;
	layout.dac_linearity[ 1 ] = 1;

	return cal_ni_generic( setup, &layout );
}

static int cal_ni_pci_6036e(calibration_setup_t *setup)
{
	ni_caldac_layout_t layout;

	if( comedi_get_version_code( setup->dev ) <= COMEDI_VERSION_CODE( 0, 7, 66 ) )
	{
		DPRINT(0, "WARNING: you need comedi driver version 0.7.67 or later\n"
		 "for this calibration to work properly\n" );
	}

	/* this is for the ad8804_debug caldac */
	init_ni_caldac_layout( &layout );
	layout.adc_pregain_offset = 0;
	layout.adc_postgain_offset = 4;
	layout.adc_pregain_offset_fine = 8;
	layout.adc_gain = 2;
	layout.dac_offset[ 0 ] = 6;
	layout.dac_gain[ 0 ] = 7;
	layout.dac_gain_fine[ 0 ] = 11;
	layout.dac_linearity[ 0 ] = 10;
	layout.dac_offset[ 1 ] = 9;
	layout.dac_gain[ 1 ] = 3;
	layout.dac_gain_fine[ 1 ] = 5;
	layout.dac_linearity[ 1 ] = 1;

	return cal_ni_generic( setup, &layout );
}

static int cal_ni_pci_6071e(calibration_setup_t *setup)
{
	ni_caldac_layout_t layout;

	if( comedi_get_version_code( setup->dev ) <= COMEDI_VERSION_CODE( 0, 7, 66 ) )
	{
		DPRINT(0, "WARNING: you need comedi driver version 0.7.67 or later\n"
		 "for this calibration to work properly\n" );
	}

	init_ni_caldac_layout( &layout );
	layout.adc_pregain_offset = 8;
	layout.adc_postgain_offset = 4;
	layout.adc_unip_offset = 7;
	layout.adc_gain = 2;
	layout.dac_offset[ 0 ] = 6;
	layout.dac_gain[ 0 ] = 11;
	layout.dac_linearity[ 0 ] = 10;
	layout.dac_offset[ 1 ] = 9;
	layout.dac_gain[ 1 ] = 5;
	layout.dac_linearity[ 1 ] = 1;
	return cal_ni_generic( setup, &layout );
}

static int cal_ni_pxi_6071e(calibration_setup_t *setup)
{
	ni_caldac_layout_t layout;

	if( comedi_get_version_code( setup->dev ) <= COMEDI_VERSION_CODE( 0, 7, 66 ) )
	{
		DPRINT(0, "WARNING: you need comedi driver version 0.7.67 or later\n"
		 "for this calibration to work properly\n" );
	}

	init_ni_caldac_layout( &layout );
	layout.adc_pregain_offset = 0;
	layout.adc_pregain_offset_fine = 8;
	layout.adc_postgain_offset = 4;
	layout.adc_gain = 2;
	layout.dac_offset[ 0 ] = 6;
	layout.dac_gain[ 0 ] = 11;
	layout.dac_linearity[ 0 ] = 10;
	layout.dac_offset[ 1 ] = 9;
	layout.dac_gain[ 1 ] = 5;
	layout.dac_linearity[ 1 ] = 1;
	return cal_ni_generic( setup, &layout );
}

static int cal_ni_at_mio_16e_10(calibration_setup_t *setup)
{
	ni_caldac_layout_t layout;

	if(comedi_get_version_code(setup->dev) <= COMEDI_VERSION_CODE(0, 7, 68))
	{
		DPRINT(0, "WARNING: you need comedi driver version 0.7.69 or later\n"
		 "for this calibration to work properly\n" );
	}
	init_ni_caldac_layout( &layout );
	layout.adc_pregain_offset = 0;
	layout.adc_pregain_offset_fine = 8;
	layout.adc_postgain_offset = 4;
	layout.adc_gain = 2;
	layout.adc_unip_offset = 7;
	layout.dac_offset[ 0 ] = 6;
	layout.dac_gain[ 0 ] = 11;
	layout.dac_linearity[ 0 ] = 10;
	layout.dac_offset[ 1 ] = 9;
	layout.dac_gain[ 1 ] = 5;
	layout.dac_linearity[1] = 1;
	return cal_ni_generic( setup, &layout );
}

static int cal_ni_pci_mio_16xe_50(calibration_setup_t *setup)
{
	ni_caldac_layout_t layout;

	init_ni_caldac_layout( &layout );
	layout.adc_pregain_offset = 8;
	layout.adc_postgain_offset = 2;
	layout.adc_gain = 0;
	layout.adc_gain_fine = 1;
	layout.adc_unip_offset = 7;
	layout.dac_offset[ 0 ] = 6;
	layout.dac_gain[ 0 ] = 4;
	layout.dac_offset[ 1 ] = 7;
	layout.dac_gain[ 1 ] = 5;

	return cal_ni_generic( setup, &layout );
}

static int cal_ni_pci_6023e(calibration_setup_t *setup)
{
	/* for comedi-0.7.65 */
	ni_caldac_layout_t layout;

	init_ni_caldac_layout( &layout );
	layout.adc_pregain_offset = 8; /* possibly wrong */
	layout.adc_pregain_offset_fine = 0;
	layout.adc_postgain_offset = 4;
	layout.adc_gain = 2;

	return cal_ni_generic( setup, &layout );
}

static int cal_ni_pci_6024e(calibration_setup_t *setup)
{
	ni_caldac_layout_t layout;

	init_ni_caldac_layout( &layout );
	layout.adc_pregain_offset = 0;
	layout.adc_postgain_offset = 4;
	layout.adc_pregain_offset_fine = 8;
	layout.adc_gain = 2;
	layout.dac_offset[ 0 ] = 6;
	layout.dac_gain[ 0 ] = 11;
	layout.dac_linearity[ 0 ] = 10;
	layout.dac_offset[ 1 ] = 9;
	layout.dac_gain[ 1 ] = 5;
	layout.dac_linearity[ 1 ] = 1;

	return cal_ni_generic( setup, &layout );
}

static int cal_ni_pci_6052e(calibration_setup_t *setup)
{
	/*
	 * This board has noisy caldacs
	 *
	 * The NI documentation says (true mb88341 addressing):
	 *   0, 8   AI pregain  (coarse, fine)
	 *   4, 12  AI postgain
	 *   2, 10  AI reference
	 *   14, 7  AI unipolar offset
	 *
	 *   0      AO0 linearity
	 *   8, 4   AO0 reference
	 *   12     AO0 offset
	 *   2      AO1 linearity
	 *   10, 6  AO1 reference
	 *   14     AO1 offset
	 *
	 *  For us, these map to (ad8804 channels)
	 *
	 *   0, 1   AI pregain  (coarse, fine)
	 *   2, 3  AI postgain
	 *   4, 5  AI reference
	 *   7  AI unipolar offset
	 *
	 *   0      AO0 linearity
	 *   1, 2   AO0 reference
	 *   3      AO0 offset
	 *   4      AO1 linearity
	 *   5, 6   AO1 reference
	 *   7      AO1 offset
	 *
	 *  or, with mb88341 channels
	 *
	 *   xxx    AO0 linearity
	 *   7, 3   AO0 reference
	 *   11     AO0 offset
	 *   1      AO1 linearity
	 *   9, 5   AO1 reference
	 *   xxx    AO1 offset
	 *
	 */
	ni_caldac_layout_t layout;

	init_ni_caldac_layout( &layout );
	layout.adc_pregain_offset = 0;
	layout.adc_postgain_offset = 2;
	layout.adc_gain = 4;
	layout.adc_unip_offset = 6;
	layout.adc_unip_offset_fine = 7;
	layout.adc_pregain_offset_fine = 1;
	layout.adc_postgain_offset_fine = 3;
	layout.adc_gain_fine = 5;

	DPRINT(0, "WARNING: you need comedi driver version 0.7.67 or later\n"
	 "for this calibration to work properly\n" );
/* this works when the first two caldacs are ad8804_debug */
	layout.dac_offset[ 0 ] = 16 + 3;
	layout.dac_gain[ 0 ] = 16 + 1;
	layout.dac_gain_fine[ 0 ] = 16 + 2;
	layout.dac_linearity[ 0 ] = 16 + 0;
	layout.dac_offset[ 1 ] = 16 + 7;
	layout.dac_gain[ 1 ] = 16 + 5;
	layout.dac_gain_fine[ 1 ] = 16 + 6;
	layout.dac_linearity[ 1 ] = 16 + 4;

	return cal_ni_generic( setup, &layout );
}

static int cal_ni_daqcard_ai_16e_4(calibration_setup_t *setup)
{
	ni_caldac_layout_t layout;

	init_ni_caldac_layout( &layout );
	layout.adc_pregain_offset = 0;
	layout.adc_postgain_offset = 1;
	layout.adc_gain = 3;
	layout.adc_unip_offset = 2;

	return cal_ni_generic( setup, &layout );
}

static int adc_offset_611x( unsigned int channel )
{
	return 2 * channel + 2;
}
static int adc_gain_611x( unsigned int channel )
{
	return 2 * channel + 1;
}
static int dac_offset_611x( unsigned int channel )
{
	return 12 + 2 + 2 * channel;
}
static int dac_gain_611x( unsigned int channel )
{
	return 12 + 1 + 2 * channel;
}
static int cal_ni_pci_611x( calibration_setup_t *setup )
{
	generic_layout_t layout;

	init_generic_layout( &layout );
	layout.adc_offset = adc_offset_611x;
	layout.adc_gain = adc_gain_611x;
	layout.dac_offset = dac_offset_611x;
	layout.dac_gain = dac_gain_611x;
	layout.adc_high_observable = ni_reference_611x;
	layout.adc_ground_observable = ni_zero_offset_611x;
	layout.dac_high_observable = ni_ao_reference_611x;
	layout.dac_ground_observable = ni_ao_zero_offset_611x;

	return generic_cal_by_channel_and_range( setup, &layout );
}

static int cal_ni_pci_mio_16e_4( calibration_setup_t *setup )
{
	ni_caldac_layout_t layout;

	init_ni_caldac_layout( &layout );
	layout.adc_pregain_offset = 8;
	layout.adc_postgain_offset = 4;
	layout.adc_gain = 2;
	layout.adc_unip_offset = 7;
	layout.dac_offset[ 0 ] = 6;
	layout.dac_gain[ 0 ] = 11;
	layout.dac_linearity[ 0 ] = 10;
	layout.dac_offset[ 1 ] = 9;
	layout.dac_gain[ 1 ] = 5;
	layout.dac_linearity[ 1 ] = 1;

	return cal_ni_generic( setup, &layout );
}

static int cal_ni_daqcard_6062e( calibration_setup_t *setup )
{
	ni_caldac_layout_t layout;

	if( comedi_get_version_code( setup->dev ) <= COMEDI_VERSION_CODE( 0, 7, 66 ) )
	{
		DPRINT(0, "WARNING: you need comedi driver version 0.7.67 or later\n"
		 "for this calibration to work properly\n" );
	}
	init_ni_caldac_layout( &layout );
	layout.adc_pregain_offset = 8;
	layout.adc_postgain_offset = 4;
	layout.adc_gain = 2;
	layout.adc_unip_offset = 7;
	layout.dac_offset[ 0 ] = 6;
	layout.dac_gain[ 0 ] = 11;
	layout.dac_linearity[ 0 ] = 10;
	layout.dac_offset[ 1 ] = 9;
	layout.dac_gain[ 1 ] = 5;
	layout.dac_linearity[ 1 ] = 1;

	return cal_ni_generic( setup, &layout );
}

static int cal_ni_daqcard_6024e( calibration_setup_t *setup )
{
	ni_caldac_layout_t layout;

	init_ni_caldac_layout( &layout );

	layout.adc_pregain_offset = 0;
	layout.adc_postgain_offset = 4;
	layout.adc_gain = 2;
	//layout.adc_unip_offset = 7;
	layout.dac_offset[ 0 ] = 6;
	layout.dac_gain[ 0 ] = 3;
	//layout.dac_linearity[ 0 ] = 10;
	layout.dac_offset[ 1 ] = 1;
	layout.dac_gain[ 1 ] = 5;
	//layout.dac_linearity[ 1 ] = 1;

	return cal_ni_generic( setup, &layout );
}

static int cal_ni_daqcard_6036e( calibration_setup_t *setup )
{
	ni_caldac_layout_t layout;

	if( comedi_get_version_code( setup->dev ) <= COMEDI_VERSION_CODE( 0, 7, 68 ) )
	{
		DPRINT(0, "WARNING: you need comedi driver version 0.7.69 or later\n"
		 "for this calibration to work properly\n" );
	}

	init_ni_caldac_layout( &layout );

	layout.adc_pregain_offset = 0;
	layout.adc_pregain_offset_fine = 8;
	layout.adc_postgain_offset = 4;
	layout.adc_gain = 2;

	layout.dac_offset[0] = 6;
	layout.dac_gain[0] = 7;
	layout.dac_gain_fine[ 0 ] = 11;
	layout.dac_linearity[0] = 10;

	layout.dac_offset[ 1 ] = 9;
	layout.dac_gain[ 1 ] = 3;
	layout.dac_gain_fine[ 1 ] = 5;
	layout.dac_linearity[1] = 1;

	return cal_ni_generic( setup, &layout );
}

static int cal_ni_at_mio_64e_3( calibration_setup_t *setup )
{
	ni_caldac_layout_t layout;

	if( comedi_get_version_code( setup->dev ) <= COMEDI_VERSION_CODE( 0, 7, 68 ) )
	{
		DPRINT(0, "WARNING: you need comedi driver version 0.7.69 or later\n"
		 "for this calibration to work properly\n" );
	}

	init_ni_caldac_layout( &layout );

	layout.adc_pregain_offset = 8;
	layout.adc_postgain_offset = 4;
	layout.adc_gain = 2;

	layout.dac_offset[0] = 6;
	layout.dac_gain[0] = 14;
	layout.dac_linearity[0] = 10;

	layout.dac_offset[ 1 ] = 9;
	layout.dac_gain[ 1 ] = 5;
	layout.dac_linearity[1] = 1;

	return cal_ni_generic( setup, &layout );
}

static void prep_adc_caldacs_generic( calibration_setup_t *setup,
	const ni_caldac_layout_t *layout, unsigned int range )
{
	int retval;

	if( setup->old_calibration == NULL )
	{
		reset_caldac( setup, layout->adc_pregain_offset );
		reset_caldac( setup, layout->adc_postgain_offset );
		reset_caldac( setup, layout->adc_gain );
		reset_caldac( setup, layout->adc_pregain_offset_fine );
		reset_caldac( setup, layout->adc_postgain_offset_fine );
		reset_caldac( setup, layout->adc_gain_fine );
		reset_caldac( setup, layout->adc_unip_offset );
		reset_caldac( setup, layout->adc_unip_offset_fine );
	}else
	{
		retval = comedi_apply_parsed_calibration( setup->dev, setup->ad_subdev,
			0, range, AREF_GROUND, setup->old_calibration );
		if( retval < 0 )
		{
			DPRINT( 0, "Failed to apply existing calibration, reseting adc caldacs.\n" );
			reset_caldac( setup, layout->adc_pregain_offset );
			reset_caldac( setup, layout->adc_postgain_offset );
			reset_caldac( setup, layout->adc_gain );
			reset_caldac( setup, layout->adc_pregain_offset_fine );
			reset_caldac( setup, layout->adc_postgain_offset_fine );
			reset_caldac( setup, layout->adc_gain_fine );
			reset_caldac( setup, layout->adc_unip_offset );
			reset_caldac( setup, layout->adc_unip_offset_fine );
		}
	}
}

static void prep_dac_caldacs_generic( calibration_setup_t *setup,
	const ni_caldac_layout_t *layout, unsigned int channel, unsigned int range )
{
	int retval;

	if( setup->da_subdev < 0 ) return;

	if( setup->old_calibration == NULL )
	{
		reset_caldac( setup, layout->dac_offset[ channel ] );
		reset_caldac( setup, layout->dac_offset_fine[ channel ] );
		reset_caldac( setup, layout->dac_gain[ channel ] );
		reset_caldac( setup, layout->dac_gain_fine[ channel ] );
		reset_caldac( setup, layout->dac_linearity[ channel ] );
	}else
	{
		retval = comedi_apply_parsed_calibration( setup->dev, setup->da_subdev,
			channel, range, AREF_GROUND, setup->old_calibration );
		if( retval < 0 )
		{
			DPRINT( 0, "Failed to apply existing calibration, reseting dac caldacs.\n" );
			reset_caldac( setup, layout->dac_offset[ channel ] );
			reset_caldac( setup, layout->dac_offset_fine[ channel ] );
			reset_caldac( setup, layout->dac_gain[ channel ] );
			reset_caldac( setup, layout->dac_gain_fine[ channel ] );
			reset_caldac( setup, layout->dac_linearity[ channel ] );
		}
	}
}

static void prep_adc_for_dac( calibration_setup_t *setup, int observable )
{
	unsigned int adc_range;
	int chanspec;

	if( observable < 0 ) return;

	chanspec = setup->observables[ observable ].observe_insn.chanspec;
	adc_range = CR_RANGE( chanspec );

	comedi_apply_parsed_calibration( setup->dev, setup->ad_subdev,
		0, adc_range, 0, setup->new_calibration );
}

static int cal_ni_generic( calibration_setup_t *setup, const ni_caldac_layout_t *layout )
{
	comedi_calibration_setting_t *current_cal;
	int retval;
	int num_ai_ranges;
	int range;
	int ai_unipolar_lowgain, ai_bipolar_lowgain;

	num_ai_ranges = comedi_get_n_ranges( setup->dev, setup->ad_subdev, 0 );
	assert( num_ai_ranges > 0 );

	ai_bipolar_lowgain = get_bipolar_lowgain( setup->dev, setup->ad_subdev );
	ai_unipolar_lowgain = get_unipolar_lowgain( setup->dev, setup->ad_subdev );

	prep_adc_caldacs_generic( setup, layout, ai_bipolar_lowgain );

	current_cal = sc_alloc_calibration_setting(setup->new_calibration);
	current_cal->subdevice = setup->ad_subdev;
	reset_caldac( setup, layout->adc_gain_fine );
	generic_do_relative( setup, current_cal, ni_zero_offset_low,
		ni_reference_low, layout->adc_gain );
	reset_caldac( setup, layout->adc_postgain_offset_fine );
	generic_do_relative( setup, current_cal, ni_zero_offset_low,
		ni_zero_offset_high, layout->adc_postgain_offset );
	generic_do_relative( setup, current_cal, ni_zero_offset_low,
		ni_zero_offset_high, layout->adc_postgain_offset_fine );
	reset_caldac( setup, layout->adc_pregain_offset_fine );
	generic_do_cal( setup, current_cal, ni_zero_offset_high, layout->adc_pregain_offset );
	generic_do_relative( setup, current_cal, ni_zero_offset_low,
		ni_reference_low, layout->adc_gain_fine );
	generic_do_cal( setup, current_cal, ni_zero_offset_high,
		layout->adc_pregain_offset_fine );
	sc_push_channel( current_cal, SC_ALL_CHANNELS );
	sc_push_aref( current_cal, SC_ALL_AREFS );
	if( layout->adc_unip_offset >= 0 )
	{
		sc_push_range( current_cal, SC_ALL_RANGES );
	}else
	{
		for( range = 0; range < num_ai_ranges; range++ )
		{
			if( is_bipolar( setup->dev, setup->ad_subdev, 0, range ) )
				sc_push_range( current_cal, range );
		}
	}

	/* do seperate unipolar calibration if appropriate */
	if( ai_unipolar_lowgain >= 0 )
	{
		current_cal = sc_alloc_calibration_setting(setup->new_calibration);
		current_cal->subdevice = setup->ad_subdev;
		if( layout->adc_unip_offset >= 0 )
		{
			reset_caldac( setup, layout->adc_unip_offset_fine );
			generic_do_cal( setup, current_cal, ni_unip_zero_offset_high,
				layout->adc_unip_offset );
			generic_do_cal( setup, current_cal, ni_unip_zero_offset_high,
				layout->adc_unip_offset_fine );
		/* if we don't have a unipolar offset caldac, do a fully
		 * independent calibration for unipolar ranges */
		}else
		{
			prep_adc_caldacs_generic( setup, layout, ai_unipolar_lowgain );
			generic_peg( setup, ni_unip_zero_offset_low,
				layout->adc_pregain_offset, 1 );
			generic_peg( setup, ni_unip_zero_offset_low,
				layout->adc_postgain_offset, 1 );
			reset_caldac(setup, layout->adc_gain_fine);
			generic_do_relative( setup, current_cal, ni_unip_zero_offset_low,
				ni_unip_reference_low, layout->adc_gain );
			reset_caldac(setup, layout->adc_postgain_offset_fine);
			generic_do_relative( setup, current_cal, ni_unip_zero_offset_low,
				ni_unip_zero_offset_high, layout->adc_postgain_offset );
			generic_do_relative( setup, current_cal, ni_unip_zero_offset_low,
				ni_unip_zero_offset_high, layout->adc_postgain_offset_fine );
			reset_caldac( setup, layout->adc_pregain_offset_fine );
			generic_do_cal( setup, current_cal, ni_unip_zero_offset_high,
				layout->adc_pregain_offset );
			generic_do_relative( setup, current_cal, ni_unip_zero_offset_low,
				ni_unip_reference_low, layout->adc_gain_fine );
			generic_do_cal( setup, current_cal, ni_unip_zero_offset_high,
				layout->adc_pregain_offset_fine );
		}
		for( range = 0; range < num_ai_ranges; range++ )
		{
			if( is_unipolar( setup->dev, setup->ad_subdev, 0, range ) )
				sc_push_range( current_cal, range );
		}
		sc_push_channel( current_cal, SC_ALL_CHANNELS );
		sc_push_aref( current_cal, SC_ALL_AREFS );
	}
	if( setup->da_subdev >= 0 && setup->do_output )
	{
		unsigned int channel, range;
		int ao_unipolar_lowgain = get_unipolar_lowgain( setup->dev, setup->da_subdev );
		int ao_bipolar_lowgain = get_bipolar_lowgain( setup->dev, setup->da_subdev );
		int num_ao_ranges;

		for( channel = 0; channel < 2; channel++ )
		{
			num_ao_ranges = comedi_get_n_ranges( setup->dev, setup->da_subdev, channel );
			prep_dac_caldacs_generic( setup, layout, channel, ao_bipolar_lowgain );
			prep_adc_for_dac( setup, ni_ao_reference( channel ) );

			current_cal = sc_alloc_calibration_setting(setup->new_calibration);
			current_cal->subdevice = setup->da_subdev;
			generic_do_linearity( setup, current_cal, ni_ao_zero_offset( channel ),
				ni_ao_mid_linearity( channel ), ni_ao_reference( channel ),
				layout->dac_linearity[ channel ] );
			reset_caldac(setup, layout->dac_offset_fine[channel]);
			generic_do_cal( setup, current_cal, ni_ao_zero_offset( channel ),
				layout->dac_offset[ channel ] );
			generic_do_cal( setup, current_cal, ni_ao_zero_offset( channel ),
				layout->dac_offset_fine[ channel ] );
			reset_caldac( setup, layout->dac_gain_fine[ channel ] );
			generic_do_cal( setup, current_cal, ni_ao_reference( channel ),
				layout->dac_gain[ channel ] );
			generic_do_cal( setup, current_cal, ni_ao_reference( channel ),
				layout->dac_gain_fine[ channel ] );
			sc_push_channel( current_cal, channel );
			for( range = 0; range < num_ao_ranges; range++ )
			{
				if( is_bipolar( setup->dev, setup->da_subdev, channel, range ) )
					sc_push_range( current_cal, range );
			}
			sc_push_aref( current_cal, SC_ALL_AREFS );

			if( ao_unipolar_lowgain >= 0 )
			{
				prep_dac_caldacs_generic( setup, layout, channel, ao_unipolar_lowgain );

				current_cal = sc_alloc_calibration_setting(setup->new_calibration);
				current_cal->subdevice = setup->da_subdev;
				generic_do_linearity( setup, current_cal, ni_ao_unip_low_linearity( channel ),
					ni_ao_unip_mid_linearity( channel ), ni_ao_unip_reference( channel ),
					layout->dac_linearity[ channel ] );
				reset_caldac( setup, layout->dac_offset_fine[ channel ] );
				generic_do_cal( setup, current_cal, ni_ao_unip_zero_offset( channel),
					layout->dac_offset[ channel ] );
				generic_do_cal( setup, current_cal, ni_ao_unip_zero_offset( channel),
					layout->dac_offset_fine[ channel ] );
				reset_caldac( setup, layout->dac_gain_fine[ channel ] );
				generic_do_cal( setup, current_cal, ni_ao_unip_reference( channel ),
					layout->dac_gain[ channel ] );
				generic_do_cal( setup, current_cal, ni_ao_unip_reference( channel ),
					layout->dac_gain_fine[ channel ] );
				sc_push_channel( current_cal, channel );
				for( range = 0; range < num_ao_ranges; range++ )
				{
					if( is_unipolar( setup->dev, setup->da_subdev, channel, range ) )
						sc_push_range( current_cal, range );
				}
				sc_push_aref( current_cal, SC_ALL_AREFS );
			}
		}
	}

	retval = write_calibration_file(setup->cal_save_file_path, setup->new_calibration);

	return retval;
}

static double ni_get_reference( calibration_setup_t *setup, int lsb_loc,int msb_loc)
{
	int lsb,msb;
	int16_t uv;
	double ref;

	lsb=read_eeprom( setup, lsb_loc );
	msb=read_eeprom( setup, msb_loc );
	assert( lsb >=0 && msb >= 0 );
	DPRINT(0,"eeprom reference lsb=%d msb=%d\n", lsb, msb);

	uv = ( lsb & 0xff ) | ( ( msb << 8 ) & 0xff00 );
	ref=5.000+1.0e-6*uv;
	DPRINT(0, "resulting reference voltage: %g\n", ref );
	if( fabs( ref - 5.0 ) > 0.005 )
		DPRINT( 0, "WARNING: eeprom indicates reference is more than 5mV away\n"
			"from 5V.  Possible bad eeprom address?\n" );

	return ref;
}

/****************
 NI 671x and 673x support
 **************/

static const int channels_per_ad8804 = 16;

static inline int ni67xx_ao_gain_caldac(unsigned int ao_channel)
{
	int ad8804_gain_channels[4] = {8, 2, 11, 5};
	int caldac_channel = ad8804_gain_channels[ao_channel % 4];
	int caldac_index = ao_channel / 4;
	/* just guessing that second ad8804 is works for ao channels 4-7
	 * the same as the first ad8804 works for ao channels 0-3 */
	return caldac_index * channels_per_ad8804 + caldac_channel;
}
static inline int ni67xx_ao_linearity_caldac(unsigned int ao_channel)
{
	int ad8804_linearity_channels[4] = {4, 10, 1, 0};
	int caldac_channel = ad8804_linearity_channels[ao_channel % 4];
	int caldac_index = ao_channel / 4;

	return caldac_index * channels_per_ad8804 + caldac_channel;
}
static inline int ni67xx_ao_offset_caldac(unsigned int ao_channel)
{
	int ad8804_offset_channels[4] = {7, 6, 9, 3};
	int caldac_channel = ad8804_offset_channels[ao_channel % 4];
	int caldac_index = ao_channel / 4;

	return caldac_index * channels_per_ad8804 + caldac_channel;
}

static int ni67xx_ao_ground_observable_index( const calibration_setup_t *setup,
	unsigned int channel, unsigned int ao_range )
{
	return 3 * channel + 0;
}

static int ni67xx_ao_mid_observable_index( const calibration_setup_t *setup,
	unsigned int channel, unsigned int ao_range )
{
	return 3 * channel + 1;
}

static int ni67xx_ao_high_observable_index( const calibration_setup_t *setup,
	unsigned int channel, unsigned int ao_range )
{
	return 3 * channel + 2;
}

static const double ni67xx_unitless_adc_offset = 0.5;

/* determine conversion factor between actual voltage and
 * interval [0,1) returned by reads from the calibration adc
 * subdevice.
 */
static double ni67xx_unitless_adc_slope(calibration_setup_t *setup)
{
	double reference_in_volts;
	double reference_unitless;
	double slope;
	comedi_insn insn;
	lsampl_t data;
	comedi_range *range;
	static const int maxdata = 0x10000;
	int retval;

	if(ni_board(setup)->ref_eeprom_lsb >= 0 &&
		ni_board(setup)->ref_eeprom_msb >= 0)
	{
		reference_in_volts = ni_get_reference(setup,
			ni_board(setup)->ref_eeprom_lsb, ni_board(setup)->ref_eeprom_msb );
	}else
	{
		DPRINT( 0, "WARNING: unknown eeprom address for reference voltage\n"
			"correction.  This might be fixable if you send us an eeprom dump\n"
			"(see the demo/eeprom_dump program).\n");
		reference_in_volts = 5.0;
	}

	memset(&insn, 0, sizeof(insn));
	insn.insn = INSN_READ;
	insn.n = 1;
	insn.subdev = setup->ad_subdev;
	insn.data = &data;
	insn.chanspec = CR_PACK(0, 0, AREF_GROUND) | CR_ALT_SOURCE;
	retval = comedi_do_insn(setup->dev, &insn);
	assert(retval >= 0);

	range = comedi_get_range(setup->dev, setup->ad_subdev, 0, 0);
	assert( range );
	reference_unitless = comedi_to_phys(data, range, maxdata);

	slope = (reference_unitless - ni67xx_unitless_adc_offset) / reference_in_volts;

	return slope;
}

/* calibration adc uses RANGE_UNKNOWN, so it will return a value from
   0.0 to 1.0 instead of a voltage, so we need to renormalize. */
static void ni67xx_set_target( calibration_setup_t *setup, int obs, double target, double slope)
{
	set_target(setup, obs, target);
	/* convert target from volts to interval [0,1) which calibration
	 * adc returns */
	setup->observables[obs].target *= slope;
	setup->observables[obs].target += ni67xx_unitless_adc_offset;
}

static void ni67xx_setup_observables( calibration_setup_t *setup )
{
	comedi_insn tmpl, po_tmpl;
	observable *o;
	int num_ao_channels;
	int i;
	double slope;

	slope = ni67xx_unitless_adc_slope(setup);

	/* calibration adc is very slow (15HZ) but accurate, so only sample a few times */
	setup->sv_order = 0;

	num_ao_channels = comedi_get_n_channels(setup->dev, setup->da_subdev);
	assert(num_ao_channels >= 0);

	memset( &tmpl, 0, sizeof(tmpl) );
	tmpl.insn = INSN_READ;
	tmpl.n = 1;
	tmpl.subdev = setup->ad_subdev;

	memset( &po_tmpl, 0, sizeof(po_tmpl) );
	po_tmpl.insn = INSN_WRITE;
	po_tmpl.n = 1;
	po_tmpl.subdev = setup->da_subdev;

	setup->n_observables = 0;

	for(i = 0; i < num_ao_channels; i++)
	{
		o = setup->observables + ni67xx_ao_ground_observable_index( setup,
			i, 0);
		o->reference_source = -1;
		assert( o->name == NULL );
		asprintf(&o->name, "dac%i ground, ground referenced", i);
		o->preobserve_insn = po_tmpl;
		o->preobserve_insn.chanspec = CR_PACK(i, 0, AREF_GROUND);
		o->preobserve_insn.data = o->preobserve_data;
		o->observe_insn = tmpl;
		o->observe_insn.chanspec = CR_PACK(i, 0, AREF_GROUND);
		ni67xx_set_target(setup, ni67xx_ao_ground_observable_index(setup, i, 0), 0.0, slope);
		setup->n_observables++;

		o = setup->observables + ni67xx_ao_mid_observable_index( setup,
			i, 0);
		o->reference_source = -1;
		assert( o->name == NULL );
		asprintf(&o->name, "dac%i mid, ground referenced", i);
		o->preobserve_insn = po_tmpl;
		o->preobserve_insn.chanspec = CR_PACK(i, 0, AREF_GROUND);
		o->preobserve_insn.data = o->preobserve_data;
		o->observe_insn = tmpl;
		o->observe_insn.chanspec = CR_PACK(i, 0, AREF_GROUND);
		ni67xx_set_target(setup, ni67xx_ao_mid_observable_index(setup, i, 0), 4.0, slope);
		setup->n_observables++;

		o = setup->observables + ni67xx_ao_high_observable_index( setup, i, 0);
		o->reference_source = -1;
		assert( o->name == NULL );
		asprintf(&o->name, "dac%i high, ground referenced", i);
		o->preobserve_insn = po_tmpl;
		o->preobserve_insn.chanspec = CR_PACK( i, 0, AREF_GROUND );
		o->preobserve_insn.data = o->preobserve_data;
		o->observe_insn = tmpl;
		o->observe_insn.chanspec = CR_PACK(i, 0, AREF_GROUND);
		ni67xx_set_target(setup, ni67xx_ao_high_observable_index(setup, i, 0), 8.0, slope);
		setup->n_observables++;
	}

	return;
}

static int cal_ni_pci_6711(calibration_setup_t *setup)
{
	generic_layout_t layout;

	if( comedi_get_version_code( setup->dev ) <= COMEDI_VERSION_CODE(0, 7, 69))
	{
		DPRINT(0, "WARNING: you need comedi driver version 0.7.69 or later\n"
		 "for this calibration to work properly\n" );
	}
	init_generic_layout( &layout );
	layout.dac_gain = ni67xx_ao_gain_caldac;
	layout.dac_linearity = ni67xx_ao_linearity_caldac;
	layout.dac_offset = ni67xx_ao_offset_caldac;
	layout.dac_high_observable = ni67xx_ao_high_observable_index;
	layout.dac_mid_observable = ni67xx_ao_mid_observable_index;
	layout.dac_ground_observable = ni67xx_ao_ground_observable_index;
	layout.dac_fractional_tolerance = get_tolerance( setup, setup->da_subdev, 1.0 );
	return generic_cal_ao(setup, &layout);
}

