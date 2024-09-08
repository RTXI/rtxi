//
//  File = bilinear.h
//

#ifndef _BILINEAR_H_
#define _BILINEAR_H_
#include "filtfunc.hpp"
#include "iir_dsgn.hpp"

IirFilterDesign BilinearTransf(const FilterTransFunc& analog_filter,
                               double sampling_interval);

#endif
