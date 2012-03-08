//
//  File = dan_pdgm.h
//                   
#ifndef _DAN_PDGM_H_
#define _DAN_PDGM_H_

#include "complex.h"
#include "psd_est.h"
#include "gen_win.h"

class DaniellPeriodogram : public PsdEstimate
{
public:
  DaniellPeriodogram( complex* big_x, 
                      double samp_intvl,
                      int big_n,
                      int fft_len,
                      GenericWindow* data_wind,
                      int big_p );
  //DumpNumeric( std::ofstream *out_stream ); 
  //DumpDecibels( std::ofstream *out_stream ); 
}; 
#endif
 