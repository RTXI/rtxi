/*
    Copyright (C) 2004 Caleb Tennis

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

*/

#include "comedilib_scxi.h"
#include <string.h>
#include <stdlib.h>
#include <byteswap.h>

const scxi_board_t scxi_boards[] = {
       { 0, "unknown\0", 2, SLOW_INTERVAL, 0, 0, 0, 0, 0, 0, 0, 0 },
       { 0xffffffff, "empty\0", 2, SLOW_INTERVAL, 0, 0, 0, 0, 0, 0, 0, 0 },
       { 0x06, "SCXI-1100\0", 1, FAST_INTERVAL, SCXI_DIO_NONE, SCXI_AIO_AI, 32,
               0x00, 0x00, 0x00, 0x00, 0x00 },
       { 0x1e, "SCXI-1102\0", 2, FAST_INTERVAL, SCXI_DIO_NONE, SCXI_AIO_AI, 32,
               0x02, 0x05, 0x01, 0x03, 0x04 },
       { 0x0a, "SCXI-1122\0", 2, FAST_INTERVAL, SCXI_DIO_NONE, SCXI_AIO_AI, 16,
               0x02, 0x01, 0x00, 0x03, 0x00 },
       //{ 0x14, "SCXI-1124\0", 2, SLOW_INTERVAL, SCXI_DIO_NONE, SCXI_AIO_AO, 6,
       //      0x02, 0x08, 0x00, 0x03, 0x00 },
};

static int scxi_identify(scxi_mod_t *mod);

static int scxi_serial_config(comedi_t *it, unsigned int clock_interval)
{
	comedi_insn insn;
	lsampl_t data[2];

	memset(&insn,0,sizeof(insn));
	insn.insn = INSN_CONFIG;
	insn.n = 2;
	insn.data = &data[0];
	insn.subdev = comedi_find_subdevice_by_type(it, COMEDI_SUBD_SERIAL, 0);
	data[0]=INSN_CONFIG_SERIAL_CLOCK;
	data[1]=clock_interval;

	return comedi_do_insn(it,&insn);
}

static int scxi_serial_readwrite(comedi_t *it, unsigned char out_bits, unsigned char *in_bits)
{
	int ret;
	comedi_insn insn;
	lsampl_t data[2];

	memset(&insn,0,sizeof(insn));

	insn.insn = INSN_CONFIG;
	insn.n = 2;
	insn.data = data;
	insn.subdev = comedi_find_subdevice_by_type(it, COMEDI_SUBD_SERIAL, 0);

	data[0]=INSN_CONFIG_BIDIRECTIONAL_DATA;
	data[1]=out_bits;

	ret = comedi_do_insn(it,&insn);

	if(ret<0) return ret;

	if(in_bits)
		*in_bits = data[1];

	return 0;
}

static int local_serial_readwrite(comedi_t *it, unsigned int subdev, unsigned int num_bits,
	unsigned int out_bits, unsigned int *in_bits)
{
	unsigned char byte_out, byte_in;
	while(num_bits > 0) {
		if(in_bits != NULL) (*in_bits) <<= 8;
		byte_out = (out_bits >> (num_bits - 8)) & 0xFF;
		scxi_serial_readwrite(it,byte_out, &byte_in);
		if(in_bits != NULL) *in_bits |= byte_in;

		num_bits -= 8;
	}
	return 0;
}

static int scxi_slot_select(comedi_t *dev, unsigned int dio_subdev, 
	unsigned int serial_subdev, unsigned short chassis_address, unsigned short module_slot)
{
	const unsigned int ssreg = ((chassis_address & 0x1f) << 4) | (module_slot & 0x0f);

	comedi_dio_write(dev, dio_subdev, SCXI_LINE_SS, 0);
	scxi_serial_config(dev, SLOT0_INTERVAL);
	local_serial_readwrite(dev, serial_subdev, 16, ssreg, NULL);
	comedi_dio_write(dev, dio_subdev, SCXI_LINE_SS, 1);
	
	return 0;
}

void comedi_scxi_close(scxi_mod_t *mod)
{
	free(mod);
}

scxi_mod_t *comedi_scxi_open(comedi_t *dev, unsigned short chassis_address, unsigned short mod_slot)
{
	scxi_mod_t *mod = NULL;
	
	mod = (scxi_mod_t *) malloc(sizeof(*mod));
	if(mod == NULL) goto Error;
	
	memset(mod, 0, sizeof(*mod));
	mod->chassis = chassis_address;
	mod->slot = mod_slot;
	
	mod->dev = dev;
	if(mod->dev == NULL) goto Error;
	
	mod->dio_subdev = comedi_find_subdevice_by_type(mod->dev, COMEDI_SUBD_DIO, 0);
	mod->ser_subdev = comedi_find_subdevice_by_type(mod->dev, COMEDI_SUBD_SERIAL, 0);
	
	comedi_dio_config(mod->dev, mod->dio_subdev, SCXI_LINE_MISO, COMEDI_INPUT);
	comedi_dio_config(mod->dev, mod->dio_subdev, SCXI_LINE_DA, COMEDI_OUTPUT);
	comedi_dio_config(mod->dev, mod->dio_subdev, SCXI_LINE_SS, COMEDI_OUTPUT);
	comedi_dio_config(mod->dev, mod->dio_subdev, SCXI_LINE_MOSI, COMEDI_OUTPUT);
		
	comedi_dio_write(mod->dev, mod->dio_subdev, SCXI_LINE_DA, 1);
	comedi_dio_write(mod->dev, mod->dio_subdev, SCXI_LINE_SS, 1);

	scxi_identify(mod);
	
	return mod;
	
Error:
	comedi_scxi_close(mod);
	return NULL;
}

static int scxi_module_select(scxi_mod_t *mod, unsigned short address)
{
	scxi_slot_select(mod->dev, mod->dio_subdev, mod->ser_subdev, mod->chassis, mod->slot);

	if(scxi_serial_config(mod->dev, scxi_boards[mod->board].clock_interval) < 0)
		return -1;

	if(scxi_boards[mod->board].modclass == 2)
		if(local_serial_readwrite(mod->dev, mod->ser_subdev, 16, address, NULL) < 0) return -1;

	if(comedi_dio_write(mod->dev, mod->dio_subdev, SCXI_LINE_DA, 0) < 0) return -1;

	return 0;
}

static int scxi_module_deselect(scxi_mod_t *mod)
{
	if(comedi_dio_write(mod->dev, mod->dio_subdev, SCXI_LINE_DA, 1) < 0) return -1;

	if(scxi_boards[mod->board].modclass == 2) {
    	if(local_serial_readwrite(mod->dev, mod->ser_subdev, 16, REG_PARK, NULL) < 0) return -1;
	}
	
	scxi_slot_select(mod->dev, mod->dio_subdev, mod->ser_subdev, mod->chassis, 0);
	
	return 0;
}

static int scxi_identify(scxi_mod_t *mod)
{
	unsigned int id, i;
	if(mod == NULL || mod->dev == NULL)
		return -1;

	scxi_slot_select(mod->dev, mod->dio_subdev, mod->ser_subdev, mod->chassis, mod->slot);

	scxi_serial_config(mod->dev, SLOW_INTERVAL);
	local_serial_readwrite(mod->dev, mod->ser_subdev, 32, 0, &id);

	if(id == 0xffffffff) {
		comedi_dio_write(mod->dev, mod->dio_subdev, SCXI_LINE_DA, 0);
		local_serial_readwrite(mod->dev, mod->ser_subdev, 32, 0, &id);
		comedi_dio_write(mod->dev, mod->dio_subdev, SCXI_LINE_DA, 1);
	}

	id = bswap_32(id);

	scxi_module_deselect(mod);
	
	for (i=0;i<n_scxi_boards; i++) {
		if (id == scxi_boards[i].device_id) {
			mod->board = i;
			break;
		}
	}

	fprintf(stderr, "name=%s, device_id = 0x%x, chassis_ad = %d, mod = %d\n",
		scxi_boards[mod->board].name, scxi_boards[mod->board].device_id, mod->chassis, mod->slot);
  
	return 0;

}

int comedi_scxi_register_readwrite(scxi_mod_t *mod, unsigned short reg_address, unsigned int num_bytes,
			    unsigned char *data_out, unsigned char *data_in)
{
	unsigned int i;
	unsigned int tmp_in, tmp_out = 0;

	if(mod == NULL || mod->dev == NULL) return -1;

	scxi_module_select(mod, reg_address);

	for(i = 0; i < num_bytes; ++i) {
		if(data_out) tmp_out = data_out[i];
		local_serial_readwrite(mod->dev, mod->ser_subdev, 8, tmp_out, &tmp_in);
		if(data_in) data_in[i] = tmp_in;
	}

	if(scxi_boards[mod->board].modclass == 1 && reg_address == 0) {
		comedi_dio_write(mod->dev, mod->dio_subdev, SCXI_LINE_SS, 1);
	}

	scxi_module_deselect(mod);
	return 0;
}
