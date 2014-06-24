//
//  File = dirform1.h
//

#ifndef _DIRFORM1_H_
#define _DIRFORM1_H_

#include "filt_imp.h"
#include "typedefs.h"

class DirectFormFir : public FilterImplementation
{
public:
  DirectFormFir( int num_taps,
                 double *coeff,
                 logical quan_enab,
                 long coeff_quan_factor,
                 long input_quan_factor);
  double ProcessSample( double input_val );
  long ProcessSample( long input_val );
  int GetNumTaps( void );

protected:
  int Num_Taps;
  long *Quan_In_Buf;
  long *Quan_Coeff;
  double *Unquan_In_Buf;
  double *Unquan_Coeff;
  logical Quan_Enab;
  int Write_Indx;
  long Input_Quan_Factor;
  double Output_Quan_Factor;
  long Long_Out_Quan_Factor;

};

#endif
