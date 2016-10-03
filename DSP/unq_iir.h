//
//  File = unq_iir.h
//

#ifndef _UNQ_IIR_H_
#define _UNQ_IIR_H_
#include "filt_imp.h"

class UnquantDirectFormIir : public FilterImplementation
{
public:
  UnquantDirectFormIir( int num_numer_coeff,
                        int num_denom_coeff,
                        double *numer_coeff,
                        double *denom_coeff);
  double ProcessSample( double input_val );
  long ProcessSample( long input_val ){return(0);};
  int GetNumNumerCoeff( void );
  int GetNumTaps( void );
  int GetNumDenomCoeff( void );

private:
  int Num_Numer_Coeff;
  int Num_Denom_Coeff;
  double *Input_Buffer;
  double *Output_Buffer;
  double *Numer_Coeff;
  double *Denom_Coeff;
  int Output_Write_Indx;
  int Input_Write_Indx;

};

#endif
