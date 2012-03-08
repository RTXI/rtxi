//
//  File = psd_est.h
//                   
#ifndef _PSD_EST_H_
#define _PSD_EST_H_
#include <fstream.h>
#include "thy_spec.h"
#include "fsk_spec.h"
#include "complex.h"

class PsdEstimate
{
public:
  PsdEstimate( int num_samps,
               double samp_intvl );
  ~PsdEstimate(){};
  void DumpNumeric( ofstream *out_stream,
                    CpfskSpectrum *ref_spect ); 
  void DumpNumeric( ofstream *out_stream ); 
  void DumpDecibels( ofstream *out_stream,
                     CpfskSpectrum *ref_spect ); 
  void DumpDecibels( ofstream *out_stream ); 
protected:
  int Num_Samps;
  double Delta_F;
  double Delta_T;
  double *Psd_Est;
}; 
#endif
 