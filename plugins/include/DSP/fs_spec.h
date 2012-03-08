//
//  File = fs_spec.h
//

#ifndef _FS_SPEC_H_
#define _FS_SPEC_H_

#include <ostream.h>
#include <istream.h>
#include <stdlib.h> 
#include "typedefs.h"
#include "misdefs.h"

class FreqSampFilterSpec
{
public:

  // constructors
  
  FreqSampFilterSpec( );
  FreqSampFilterSpec( const FreqSampFilterSpec &filter_spec);
  FreqSampFilterSpec( int band_config,
                      int fir_type,
                      int num_taps,
                      double *des_mag_resp);
                      
  FreqSampFilterSpec( std::istream &uin,
                      ostream &uout);

  logical IsOptimizeEnabled(void);
  // methods to read specification parameters
  
  int GetNumTaps(void);
  int GetBandConfig(void);
  int GetN1(void);
  int GetN2(void);
  int GetN3(void);
  int GetN4(void); 
  int GetFirType(void);
  int FreqSampFilterSpec::GetNumTransSamps(void);
  double GetMagRespSamp(int);
  
  // methods to set specification parameters
  
  void SetNumTaps(int);
  void SetBandConfig(int);
  void SetN1(int);
  void SetN2(int);
  void SetN3(int);
  void SetN4(int); 
  void SetFirType(int);
  void SetDbScaleEnabled(int db_scale_enabled);
  void SetTrans(double transition_value);
  void SetTrans( double *origins, 
                 double *slopes, 
                 double rho);
  void SetMagRespSamp( int samp_indx, double value); 
  
  friend class FreqSampFilterDesign;
  friend class LinearPhaseFirResponse;
  
private:
  
  //
  // filter band configuration: 1 = lowpass,  2 = highpass,
  //                            3 = bandpass, 4 = bandstop
  int Band_Config;
  
  // type of specified frequencies:
  //                  0 = discrete frequencies specified via indices
  //                  1 = normalized for folding frequency of PI
  //                  2 = actual frequencies
  int Freq_Type;
  logical Freqs_Specif_In_Hz;
  double Samp_Freq;
  logical Cutoff_Is_Half_Amp;
  double Lambda_Numer, Lambda_Denom;
  double Lambda;
  
  int Num_Taps;  
  int Fold_Indx;
  
  int Fir_Type;
  logical Optimize_Enabled;

  int Num_Trans_Samps;  
  int N1;
  int N2;
  int N3;
  int N4;
  
  int Db_Scale_Enabled;
  double* Des_Mag_Resp;
  

};

#endif
