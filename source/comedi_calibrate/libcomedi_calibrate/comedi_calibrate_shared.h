/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License as        *
 *   published by                                                          *
 *   the Free Software Foundation; either version 2.1 of the License, or   *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef __COMEDI_CALIBRATE_SHARED_H_
#define __COMEDI_CALIBRATE_SHARED_H_

#include <comedilib.h>

#ifdef __cplusplus
extern "C" {
#endif

/* saving calibrations to file */
static const int SC_ALL_CHANNELS = -1;
static const int SC_ALL_RANGES = -1;
static const int SC_ALL_AREFS = -1;

int write_calibration_file(const char *file_path, const comedi_calibration_t *calibration);
comedi_calibration_setting_t* sc_alloc_calibration_setting(comedi_calibration_t *calibration);
void sc_push_caldac(comedi_calibration_setting_t *saved_cal, comedi_caldac_t caldac);
void sc_push_channel(comedi_calibration_setting_t *saved_cal, int channel);
void sc_push_range(comedi_calibration_setting_t *saved_cal, int range);
void sc_push_aref(comedi_calibration_setting_t *saved_cal, int aref);

#ifdef __cplusplus
}
#endif

#endif //COMEDI_CALIBRATE_SHARED

