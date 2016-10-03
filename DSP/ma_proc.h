//
//  File = ma_proc.h
//

#ifndef _MA_PROC_H_
#define _MA_PROC_H_

//#include "complex.h"
//#include "cmpx_vec.h"
//#include "typedefs.h"

template < class T >
class MaProcess
{
public:
  MaProcess( void );
  ~MaProcess( void );

  void DumpParameters(std::ostream& uout);

  T* OutputSequence( long noise_seed,
                     int seq_len );
protected:
  int Ma_Order;
  T *B_Coeffs;
  T *Old_Input;
  long Noise_Seed;
  double Drv_Noise_Var;
};

#endif
