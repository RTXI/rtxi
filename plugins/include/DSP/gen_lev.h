//
// file = gen_lev.h
//

#ifndef _GEN_LEV_H
#define _GEN_LEV_H

#include "complex.h"

 int GeneralizedLevinson( double *acf,
                          int ar_ord,
                          int ma_ord,
                          double epsilon,
                          double *a_vec);

 int GeneralizedLevinson( complex *acf,
                          int ar_ord,
                          int ma_ord,
                          double epsilon,
                          complex *a_vec);

#endif //_GEN_LEV_H