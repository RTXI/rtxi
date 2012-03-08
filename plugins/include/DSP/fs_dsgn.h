//
//  File = fs_dsgn.h
//

#ifndef _FS_DSGN_H_
#define _FS_DSGN_H_ 

#include <fstream>
#include "fir_dsgn.h"
#include "fs_spec.h"
#include "gen_win.h"

class FreqSampFilterDesign  : public FirFilterDesign
{
public:

  // constructors
  
  FreqSampFilterDesign( );
  FreqSampFilterDesign( int band_config,
                        int fir_type,
                        int num_taps,
                        double *imp_resp_coeff);
                        
  FreqSampFilterDesign( FreqSampFilterSpec &filter_spec );
  
  // methods to modify design
  
  // methods to read design parameters
  
  void ComputeCoefficients( FreqSampFilterSpec *filter_spec);
  
  friend class LinearPhaseFirResponse;
  friend class FreqSampFilterResponse;
  
private:

  FreqSampFilterSpec *Filter_Spec;
  
  //
  // filter band configuration: 1 = lowpass,  2 = highpass,
  //                            3 = bandpass, 4 = bandstop
  int Band_Config;
  
  //int Num_Taps;
  
  int Fir_Type;
  
  //double* Imp_Resp_Coeff;
};

#endif
