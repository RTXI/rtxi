//
//  File = anlg_rcn.h
//

#ifndef _ANLG_RCN_H_
#define _ANLG_RCN_H_ 

#include <fstream>
#include "typedefs.h"


class AnalogReconst
{
public:
  
  //---------------------
  // default constructor
    
  AnalogReconst( double samp_intvl,
                 int anlg_intrp_rate,
                 int num_signif_sidelobes,
                 int num_analog_samps);

  ~AnalogReconst( void);
  
  void AddSample(double new_samp);

  void DumpResult(std::ofstream* out_file);

  void CopyResult( double* output_array );

  double FindPeak( void );
    
  double FindPeakMag( void );

  double CosineCorrelate( double norm_freq, 
                          double phase_offset,
                          double cycles_per_correl);

    
private:
  double Delta_Arg;
  double *Analog_Signal;
  int Curr_Samp;
  int Num_Analog_Samps;
  int Max_Offset;
  int Analog_Interp_Rate;
  double Interp_Samp_Intvl;
  
};

#endif
