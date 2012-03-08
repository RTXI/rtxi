//
//  File = mr_lpf.cpp
//
#include <stdlib.h> 
#include <fstream>
#include "dirform1.h"
#include "fir_dsgn.h"
#include "mr_lpf.h"
   

#ifdef _DEBUG
 extern std::ofstream DebugFile;
#endif

 MultirateLowpass::MultirateLowpass( FirFilterDesign *dec_proto_filt,
                                     FirFilterDesign *int_proto_filt,
                                     int decim_rate,
                                     logical quan_enab,
                                     long input_quan_fact,
                                     long coef_quan_fact)
 {
  int poly_filt_len;
  int proto_len;
  int rho;

  Dec_Rate = decim_rate;
  Int_Rate = decim_rate;
  Quan_Enab = quan_enab;
  Dec_Out = 0.0;
  Int_In = 0.0;
  //Dec_Rho = Dec_Rate - 1;
  Dec_Rho = 0;
  Int_Rho = 0;
  
  proto_len = dec_proto_filt->GetNumTaps();
  Num_Taps = 2 * proto_len;

  //--------------------------------------------------
  // Separate the prototype filter coefficients
  // into M sets of N/M coefficients each for
  // the M polyphase filters in the decimator
  //
  poly_filt_len = proto_len/decim_rate;
  double* coef_subset = new double[poly_filt_len];
  Dec_Filt = (FilterImplementation**)new long[Dec_Rate];

  for(rho=0; rho<Dec_Rate; rho++)
  {
    dec_proto_filt->ExtractPolyphaseSet( coef_subset,
                                     Dec_Rate,
                                     rho);
    Dec_Filt[rho] = new DirectFormFir( poly_filt_len,
                                       coef_subset,
                                       quan_enab,
                                       coef_quan_fact,
                                       input_quan_fact);
  }
  //--------------------------------------------------
  // Separate the prototype filter coefficients
  // into M sets of N/M coefficients each for
  // the M polyphase filters in the interpolator
  //
  Int_Filt = (FilterImplementation**)new long[Int_Rate];

  for(rho=0; rho<Int_Rate; rho++)
  {
    int_proto_filt->ExtractPolyphaseSet( coef_subset,
                                         Int_Rate,
                                         rho);
    Int_Filt[rho] = new DirectFormFir( poly_filt_len,
                                       coef_subset,
                                       quan_enab,
                                       coef_quan_fact,
                                       input_quan_fact);
  }
  delete[] coef_subset;
  return;
}
 //==========================================================
 MultirateLowpass::~MultirateLowpass( void )
 {
   int rho;
   for(rho=0; rho<Dec_Rate; rho++) {
     delete Dec_Filt[rho];
   }
   for(rho=0; rho<Int_Rate; rho++) {
     delete Int_Filt[rho];
   }
 }
//----------------------------------------------------------
double MultirateLowpass::ProcessSample( double input_val)
{
  double output_val;
  //-----------------------------------------------------
  //  Decimator section
  //
  Dec_Out += (Dec_Filt[Dec_Rho])->ProcessSample(input_val);
  Dec_Rho--;
  if(Dec_Rho < 0) {
    Dec_Rho = Dec_Rate - 1;
    Int_In = Dec_Out;
    Dec_Out = 0.0;
  }
  //-----------------------------------------------
  //  Interpolator section
  //
  output_val = Int_Rate * 
                   (Int_Filt[Int_Rho])->ProcessSample(Int_In);
  Int_Rho++;
  if(Int_Rho >= Int_Rate) {
    Int_Rho = 0;
  }
  return(output_val);
}  
//----------------------------------------------------------
long MultirateLowpass::ProcessSample( long input_val)
{
  long output_val;
  //-----------------------------------------------------
  //  Decimator section
  //
  Quan_Dec_Out += (Dec_Filt[Dec_Rho])->ProcessSample(input_val);
  Dec_Rho--;
  if(Dec_Rho < 0) {
    Dec_Rho = Dec_Rate - 1;
    Quan_Int_In = Quan_Dec_Out;
    Quan_Dec_Out = 0;
  }
  //-----------------------------------------------
  //  Interpolator section
  //
  output_val = Int_Rate * 
                (Int_Filt[Int_Rho])->ProcessSample(Quan_Int_In);
  Int_Rho++;
  if(Int_Rho >= Int_Rate) {
    Int_Rho = 0;
  }
  return(output_val);
}  
//--------------------------------------------------
//
int MultirateLowpass::GetNumTaps(void)
{
  return(Num_Taps);
}
