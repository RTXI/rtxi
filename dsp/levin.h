//
// file = levin.h
//

#ifndef _LEVIN_H
#define _LEVIN_H

#include "complex.h"

 int LevinsonRecursion( double *toeplitz,
                        int ar_order,
                        double *avec,
                        double *ar_drv_var);

 int LevinsonRecursion( complex *toeplitz,
                        int ar_order,
                        complex *avec,
                        double *ar_drv_var);

#endif //_LEVIN_H