 //
 // File = welcpdgm.h
 //                    
 
 #ifndef _WELCPDGM_H_
 #define _WELCPDGM_H_
 
 #include "complex.h"
 #include "sig_src.h"
 #include "gen_win.h"
 #include "psd_est.h"
 
 class WelchPeriodogram : public PsdEstimate
 {
 public:
   WelchPeriodogram( SignalSource* signal_source,
                     double samp_intvl,
                     int num_samps_per_seg,
                     int shift_between_segs,
                     int fft_len,
                     GenericWindow* data_wind,
                     int num_segs_to_avg );
 };
 #endif  // _WELCPDGM_H_
