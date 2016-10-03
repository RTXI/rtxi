//
//  File = ar_proc.h
//

#ifndef _AR_PROC_H_
#define _AR_PROC_H_

#include "complex.h"

template< class T >
class ArProcess
{
public:
  ArProcess( void );

  ~ArProcess(void);
  
  void DumpParameters( std::ostream& uout);

  void GetParameters( int *ar_order, T *a_coeff);

  T* OutputSequence( long noise_seed,
                     int seq_len );
  T NextSample( long noise_seed_init );
  T NextSample( void );
  T GetVariance( void );
  T GetMean(void);
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
