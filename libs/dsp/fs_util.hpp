//
// File = fs_util.h
//

#ifndef _FS_UTIL_H_
#define _FS_UTIL_H_

#include "fs_spec.h"
#include "typedefs.h"

void DumpRectangCompon(double* origins, double* slopes, int num_trans_samps,
                       double x);

void pause(logical pause_enabled);

#endif
