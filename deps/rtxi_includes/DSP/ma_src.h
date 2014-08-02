//
//  File = ma_src.h
//

#ifndef _MA_SRC_H_
#define _MA_SRC_H_

#include "ma_proc.h"

template <class T>
class MaSource : public MaProcess<T>
{
public:
  MaSource( int ma_order, 
            T *b_coeffs,
            double drv_noise_var );
  ~MaSource(void);
};

#endif
