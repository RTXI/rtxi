//
//  File = lin_resp.h
//

#ifndef _LIN_RESP_H_
#define _LIN_RESP_H_

#include "fir_resp.h"
#include "lin_dsgn.h"

class LinearPhaseFirResponse : public FirFilterResponse
{
public:

  //--------------------------------------------------------
  // default constructor
  
  LinearPhaseFirResponse( );
  
  //----------------------------------------------------------
  //  alternate constructor that provides interactive
  //  setting of configuration parameters
  
  LinearPhaseFirResponse( LinearPhaseFirDesign *filter_design,
                          std::istream& uin,
                          std::ostream& uout );
 
  //----------------------------------------
  // method to compute magnitude response 
  // using formulas from Table 12.2
  
  void ComputeMagResp( void );
  
  //--------------------------------
  // method not used until chapter 15
  // omit from Listing 12.x and introdice in chap 15
  double GetStopbandPeak(void);
  
                         
private:

  // filter band configuration: 1 = lowpass,  2 = highpass,
  //                            3 = bandpass, 4 = bandstop
  //int Band_Config;
  //int Fir_Type;
  
  // these vars are used for band searching in chap15
  //int N1, N2, N3, N4;
};

#endif
