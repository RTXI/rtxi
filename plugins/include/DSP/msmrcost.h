//
// File = msmrcost.h
//
#ifndef _MSMRCOST_H_
#define _MSMRCOST_H_  
void MultistageMultirateCost( int num_stages,
                              int* stage_factor,
                              double highest_samp_rate,
                              double passband_edge_freq,
                              double passband_ripple,
                              double stopband_ripple,
                              int num_taps_must_be_odd);
#endif //_MSMRCOST_H_