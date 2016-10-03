//
//  File = bilinear.h
//

#ifndef _BILINEAR_H_
#define _BILINEAR_H_
#include "iir_dsgn.h"
#include "filtfunc.h"

IirFilterDesign* BilinearTransf(
                     FilterTransFunc* analog_filter,
                     double sampling_interval); 

#endif                               
