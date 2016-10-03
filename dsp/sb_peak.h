//
// File = sb_peak.h
//

#ifndef _SB_PEAK_H_
#define _SB_PEAK_H_ 
#include "fs_spec.h"

double FindStopbandPeak(  FreqSampFilterSpec *filt_config,
                          int numPts,
                          double H[]);
#endif
