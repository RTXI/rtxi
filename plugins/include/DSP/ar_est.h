//
//  File = ar_est.h
//

#ifndef _AR_EST_H_
#define _AR_EST_H_

#include "ar_proc.h"

template< class T>
class ArEstimate : public ArProcess<T>
{
public:

  ArEstimate( int est_ar_order,
              T* sig_seq,
              int seq_len);

  ~ArEstimate(void);
 
  void DumpParameters( std::ostream& uout);
};

#endif
