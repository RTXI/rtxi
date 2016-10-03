//
//  File = armaproc.h
//

#ifndef _ARMAPROC_H_
#define _ARMAPROC_H_

#include "complex.h"
#include "typedefs.h"

template <class T>
class ArmaProcess
{
public:
  ArmaProcess( void );

  ~ArmaProcess( void);

  void DumpParameters(std::ostream& uout);

  T* OutputSequence( long noise_seed,
                     int seq_len );
  void GetParameters( int* ar_order,
                      T* a_coeff,
                      int* ma_order,
                      T* b_coeff);
  double GetDrivingVariance( void );
protected:
  int Ar_Order;
  int Ma_Order;
  T *A_Coeffs;
  T *B_Coeffs;
  T *Old_Input;
  T *Old_Output;
  long Noise_Seed;
  double Drv_Noise_Var;
};

#endif
