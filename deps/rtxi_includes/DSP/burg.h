//
//  File = burg.h
//

#ifndef _BURG_H_
#define _BURG_H_

#include "complex.h"
#include "typedefs.h"
#include "ar_proc.h"

template< class T>
class BurgMethod
{
public:

  BurgMethod( int est_ar_order,
              T* sig_seq,
              int seq_len);

  ~BurgMethod(void);

  void DumpParameters( std::ostream& uout);
  void GetParameters( int *ar_order, T *a_coeff);
  double GetDrivingVariance(void);

protected:
  int Ar_Order;
  T *A_Coeffs;
  T *Old_Output;
  long Noise_Seed;
  double Drv_Noise_Var;
  T Sum_Wgn_Samps;
  T Sum_Wgn_Sqrs;
  T Sum_Samps;
  T Sum_Squares;
  int Num_Samps;
};

#endif
