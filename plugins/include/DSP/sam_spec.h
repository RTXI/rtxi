//
//  File = sam_spec.h
//                   
#ifndef _SAM_SPEC_H_
#define _SAM_SPEC_H_

#include "complex.h"
#include "psd_est.h"
class SampleSpectrum : public PsdEstimate
{
public:
  SampleSpectrum( complex* big_x, 
                  double samp_intvl,
                  int big_n );
}; 
#endif
 