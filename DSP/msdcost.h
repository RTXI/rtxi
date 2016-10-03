//
// File = msdcost.h
//
#ifndef _MSDCOST_H_
#define _MSDCOST_H_  
void MultistageDecimCost( int num_stages,
                          int* stage_decim_rate,
                          double orig_samp_rate,
                          double passband_edge_freq,
                          double passband_ripple,
                          double stopband_ripple,
                          int num_taps_must_be_odd);
#endif //_MSDCOST_H_