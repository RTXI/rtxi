//
//  File = mr_lpf.h
//

#ifndef _MR_LPF_H_
#define _MR_LPF_H_

#include "filt_imp.h"
#include "fir_dsgn.h"
#include "typedefs.h"

class MultirateLowpass : public FilterImplementation
{
public:
 MultirateLowpass( FirFilterDesign *dec_proto_filt,
                   FirFilterDesign *int_proto_filt,
                   int decim_rate,
                   logical quan_enab,
                   long input_quant_factor,
                   long coeff_quant_factor);
 ~MultirateLowpass();
 double ProcessSample( double input_val );
 long ProcessSample( long input_val );
 int GetNumTaps( void );

private:
  int Dec_Rate;
  int Dec_Rho;
  FilterImplementation** Dec_Filt;

  int Int_Rate;
  int Int_Rho;
  FilterImplementation** Int_Filt;

  double Dec_Out;
  double Int_In;
  long Quan_Dec_Out;
  long Quan_Int_In;
  logical Quan_Enab;
  int Num_Taps;
};

#endif
