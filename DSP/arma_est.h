//
//  File = arma_est.h
//

#ifndef _ARMA_EST_H_
#define _ARMA_EST_H_

#include "armaproc.h"

template <class T>
class ArmaEstimate : public ArmaProcess<T>
{
public:
  ArmaEstimate( int est_ar_order,
                int est_ma_order,
                int durbin_ar_order,
                T* sig_seq,
                int seq_len);

};

#endif
