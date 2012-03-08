//
//  File = ar_src.h
//

#ifndef _AR_SRC_H_
#define _AR_SRC_H_

#include "ar_proc.h"

template< class T > 
class ArSource : public ArProcess<T>
{
public:
  ArSource( int ar_order, 
            T *a_coeffs,
            double drv_noise_var );

  ~ArSource(void);
  
};

#endif
