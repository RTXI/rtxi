//
// File = bartpdgm.cpp
//

#include <cmath>

#include "bartpdgm.h"

#include "complex.h"
#include "fft.h"

BartlettPeriodogram::BartlettPeriodogram(SignalSource* signal_source,
                                         double samp_intvl,
                                         int num_samps_per_seg,
                                         int fft_len,
                                         GenericWindow* data_wind,
                                         int num_segs_to_avg)
    : PsdEstimate(num_samps_per_seg, samp_intvl)
{
  double* window_seq = nullptr;
  std::vector<complex> time_seg(static_cast<size_t>(num_samps_per_seg));
  std::vector<complex> freq_seg(static_cast<size_t>(fft_len));
  double scale_factor = NAN;
  std::vector<double>& psd_est = GetPsdEstimate();

  for (size_t samp_idx = 0; samp_idx < static_cast<size_t>(fft_len); samp_idx++)
  {
    psd_est[samp_idx] = 0.0;
  }

  if (data_wind != nullptr) {
    window_seq = data_wind->GetDataWindow();
  }

  for (size_t seg_idx = 0; seg_idx < static_cast<size_t>(num_segs_to_avg);
       seg_idx++)
  {
    signal_source->GetNextSegment(time_seg.data(), num_samps_per_seg);
    if (data_wind != nullptr) {
      for (size_t samp_idx = 0;
           samp_idx < static_cast<size_t>(num_samps_per_seg);
           samp_idx++)
      {
        time_seg[samp_idx] *= window_seq[samp_idx];
      }
    }

    fft(time_seg.data(), freq_seg.data(), num_samps_per_seg, fft_len);

    for (size_t samp_idx = 0; samp_idx < static_cast<size_t>(fft_len);
         samp_idx++)
    {
      psd_est[samp_idx] += mag_sqrd(freq_seg[samp_idx]);
    }
  }
  scale_factor = num_segs_to_avg * num_samps_per_seg / samp_intvl;
  for (size_t samp_idx = 0; samp_idx < static_cast<size_t>(fft_len); samp_idx++) {
    psd_est[samp_idx] /= scale_factor;
  }
}

