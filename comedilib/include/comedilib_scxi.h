/*
    include/comedilib_scxi.h
    header file for the comedi scxi library routines

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

#ifndef _COMEDILIB_SCXI_H
#define _COMEDILIB_SCXI_H

#include <comedilib.h>

#define SLOT0_INTERVAL  1200
#define FAST_INTERVAL   1200
#define MEDIUM_INTERVAL 10000
#define SLOW_INTERVAL   30000

#define SCXI_LINE_MOSI 0
#define SCXI_LINE_DA   1
#define SCXI_LINE_SS   2
#define SCXI_LINE_MISO 4

#define SCXI_DIO_NONE 0
#define SCXI_DIO_DO   1
#define SCXI_DIO_DI   2

#define SCXI_AIO_NONE 0
#define SCXI_AIO_AO   1
#define SCXI_AIO_AI   2

#define REG_PARK 0x0FFFF

struct scxi_board_struct {
	unsigned int device_id;
	char name[12];
	int modclass;
	unsigned int clock_interval;
	int dio_type, aio_type, channels, status_reg, data_reg;
	int config_reg, eeprom_reg, gain_reg;
};

typedef struct scxi_board_struct scxi_board_t;

#define n_scxi_boards ((sizeof(scxi_boards)/sizeof(scxi_boards[0])))

struct scxi_module_struct {
	comedi_t *dev;
	unsigned int board;
	unsigned int dio_subdev, ser_subdev;
	unsigned int chassis, slot;
};

typedef struct scxi_module_struct scxi_mod_t;

#ifdef __cplusplus
extern "C" {
#endif

void comedi_scxi_close(scxi_mod_t *mod);
scxi_mod_t *comedi_scxi_open(comedi_t *dev, unsigned short chassis_address, unsigned short mod_slot);
int comedi_scxi_register_readwrite(scxi_mod_t *mod, unsigned short reg_address, unsigned int num_bytes,
			    unsigned char *data_out, unsigned char *data_in);

#ifdef __cplusplus
}
#endif

#endif	// _COMEDILIB_SCXI_H
