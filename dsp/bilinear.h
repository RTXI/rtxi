//
//  File = bilinear.h
//

#ifndef _BILINEAR_H_
#define _BILINEAR_H_
#include "filtfunc.h"
#include "iir_dsgn.h"

IirFilterDesign* BilinearTransf(FilterTransFunc* analog_filter,
                                double sampling_interval);

#endif
