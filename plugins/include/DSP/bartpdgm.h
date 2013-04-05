//
// File = bartpdgm.h
//                    

#ifndef _BARTPDGM_H_
#define _BARTPDGM_H_

#include "sig_src.h"
#include "gen_win.h"
#include "psd_est.h"

class BartlettPeriodogram : public PsdEstimate
{
public:
  BartlettPeriodogram( SignalSource* signal_source,
                       double samp_intvl,
                       int num_samps_per_seg,
                       int fft_len,
                       GenericWindow* data_wind,
                       int num_segs_to_avg );
};
#endif  // _BARTPDGM_H_
