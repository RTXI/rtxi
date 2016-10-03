//
//  File = iir_resp.h
//

#ifndef _IIR_RESP_H_
#define _IIR_RESP_H_

#include "iir_dsgn.h"
#include "complex.h"

class IirFilterResponse
{
public:

  //-------------------------------------------------
  // constructor with all configuration parameters 
  // passed as input arguments

  IirFilterResponse( IirFilterDesign *filter_design,
                     int num_resp_pts,
                     int db_scale_enabled,
                     int normalize_enabled,
                     char* resp_file_name );
  
  //--------------------------------------------------
  //  alternate constructor with configuration 
  //  parameters obtained interactively through 
  //  streams uin and uout
                     
  IirFilterResponse( IirFilterDesign *filter_design,
                     std::istream& uin,
                     std::ostream& uout );
  
  //--------------------------------------
  // method to compute frequency response
  // from the data set up by constructor
  
  virtual void ComputeResponse( void );
  
  //---------------------------------------
  // method to normalize magnitude response
  
  void NormalizeResponse( void );
  
  //-----------------------------
  // method that returns pointer
  // to an array that holds the
  // completed magnitude response
  // (ordinates only)
                         
  double* GetMagResp( void);
  
  //------------------------------------
  // method that outputs magnitude
  // response to the stream 
  // pointed-to by Response_File
  // (ordinates and normalized abscissae
  
  void DumpMagResp( void );

  double GetIntervalPeak(int beg_indx, int end_indx);
  
  
protected:

  IirFilterDesign *Filter_Design;
  int Num_Resp_Pts;
  int Db_Scale_Enabled;
  int Normalize_Enabled;
  std::ofstream *Response_File;
  int Num_Numer_Coeffs; 
  int Num_Denom_Coeffs; 
  complex* Freq_Resp;
  double* Mag_Resp;
  double* Phase_Resp;
  
};

#endif
