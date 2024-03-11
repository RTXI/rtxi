//
// File = bartpdgm.h
//

#ifndef BARTLETT_PERIODOGRAM_HPP
#define BARTLETT_PERIODOGRAM_HPP

#include "generic_window.hpp"
#include "psd_estimate.hpp"
#include "signal_source.hpp"

class BartlettPeriodogram : public PsdEstimate
{
public:
  BartlettPeriodogram(SignalSource* signal_source,
                      double samp_intvl,
                      int num_samps_per_seg,
                      int fft_len,
                      const rtxi::dsp::GenericWindow& data_wind,
                      int num_segs_to_avg);
};
#endif  // _BARTPDGM_H_
