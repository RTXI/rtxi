//
//  File = fsk_spec.h
//                   
#ifndef _FSK_SPEC_H_
#define _FSK_SPEC_H_

#include "thy_spec.h"

class CpfskSpectrum : public TheoreticalSpectrum
{
public:
  CpfskSpectrum( int big_m, 
                 double f_d, 
                 double big_t); 
  double GetPsdValue( double freq ); 
private:
  int Big_M;
  double Big_T;
  double Freq_Dev;
};
#endif
 