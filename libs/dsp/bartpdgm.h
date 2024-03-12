//
// File = bartpdgm.h
//

#ifndef _BARTPDGM_H_
#define _BARTPDGM_H_

#include "gen_win.h"
#include "psd_est.h"
#include "sig_src.h"

/*!
  * Bartlett Periodogram
  */
class BartlettPeriodogram : public PsdEstimate
{
public:

  /*!
    * Build a Bartlett periodogram
    *
    * \param signal_source Pointer to SignalSource class where signal is generated
    * \param samp_intvl Value describing the sample interval
    * \param num_samps_per_seg Number of samples per segment to use for the periodogram
    * \param fft_len Length of the fft output
    * \param data_wind Pointer to GenericWindow class representing the window to apply
    * \param num_segs_to_avg Number of segments to average through
    *
    * \sa GenericWindow::GenericWindow()
    * \sa SignalSource::SignalSource()
    */
  BartlettPeriodogram(SignalSource* signal_source, double samp_intvl,
                      int num_samps_per_seg, int fft_len,
                      GenericWindow* data_wind, int num_segs_to_avg);
};
#endif // _BARTPDGM_H_
