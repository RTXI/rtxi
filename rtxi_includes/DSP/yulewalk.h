//
//  File = yulewalk.h
//

#ifndef _YULEWALK_H_
#define _YULEWALK_H_

#include "complex.h"
#include "typedefs.h"

template <class T>
class YuleWalker
{
public:
  YuleWalker( T *signal,
              int seq_len,
              int ar_ord,
              T *a_vec,
              double *drv_noise_var,
              int *err_stat);

  YuleWalker( T *toep_corr_mtx,
              int ar_ord,
              T *a_vec,
              double *drv_noise_var,
              int *err_stat);
};

#endif
