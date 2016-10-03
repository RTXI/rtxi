//
//  File = fs_resp.h
//

#ifndef _FS_RESP_H_
#define _FS_RESP_H_

#include "fs_dsgn.h"

class FreqSampFilterResponse
{
public:

  // constructors
  
  FreqSampFilterResponse( );
  FreqSampFilterResponse( FreqSampFilterDesign *filter_design,
                          int num_resp_pts,
                          int db_scale );
  
  // destructor
  
  ~FreqSampFilterResponse( );

  // methods
 
  double GetStopbandPeak(void);
  
  void ComputeMagResp( FreqSampFilterDesign *filter_design,
                       int db_scale);
                         
  double* GetMagResp( void);
  
  void DumpMagResp( std::ofstream* output_stream);
  
  void NormalizeResponse(int db_scale);
  
private:

  //
  // filter band configuration: 1 = lowpass,  2 = highpass,
  //                            3 = bandpass, 4 = bandstop
  int Band_Config;
  int Db_Scale_Enabled;
  int Num_Taps; 
  int Fir_Type;
  int N1, N2, N3, N4;
  int Num_Resp_Pts;
  
  double* Mag_Resp;
};

#endif
