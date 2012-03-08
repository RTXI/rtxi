//
//  File = ma_est.h
//

#ifndef _MA_EST_H_
#define _MA_EST_H_

//#include "complex.h"
//#include "cmpx_vec.h"
//#include "typedefs.h"
#include "ma_proc.h"

template < class T >
class MaEstimate : public MaProcess<T>
{
public:
  MaEstimate( int ma_order,
              int durbin_ar_order,
              T* sig_2,
              int seq_len);

};

#endif
