//
//  File = arma_src.h
//

#ifndef _ARMA_SRC_H_
#define _ARMA_SRC_H_

#include "armaproc.h"

template <class T>
class ArmaSource : public ArmaProcess<T>
{
public:
  ArmaSource( int ar_order,
              T *a_coeffs,
              int ma_order,
              T *b_coeffs,
              double drv_noise_var );

};

#endif
