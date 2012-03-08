//
//  File = dir1_iir.h
//

#ifndef _DIR1_IIR_H_
#define _DIR1_IIR_H_
#include "filt_imp.h"

class DirectFormIir : public FilterImplementation
{
public:
  DirectFormIir( int num_numer_coeff,
                 int num_denom_coeff,
                 double *numer_coeff,
                 double *denom_coeff,
                 long coeff_quan_factor,
                 long input_quan_factor);
  double ProcessSample( double input_val );
  long ProcessSample( long input_val );
  int GetNumNumerCoeff( void );
  int GetNumTaps( void );
  int GetNumDenomCoeff( void );

private:
  int Num_Numer_Coeff;
  int Num_Denom_Coeff;
  long *Input_Buffer;
  long *Output_Buffer;
  long *Quan_Numer_Coeff;
  long *Quan_Denom_Coeff;
  int Output_Write_Indx;
  int Input_Write_Indx;
  long Input_Quan_Factor;
  long Coeff_Quan_Factor;
  double Output_Quan_Factor;

};

#endif
