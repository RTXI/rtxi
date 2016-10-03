//
//  File = lms_filt.h
//

#ifndef _LMS_FILT_H_
#define _LMS_FILT_H_

#include "adap_fir.h"
#include "typedefs.h"

class LmsFilter : public AdaptiveFir
{
public:
  LmsFilter( int num_taps,
             double *coeff,
             double mu,
             logical quan_enab,
             long coeff_quan_factor,
             long input_quan_factor,
             int tap_for_trans,
             int secondary_tap,
             int transient_len);
  double UpdateTaps( double true_samp,
                     double estim_samp,
                     logical trans_save_enabled );
  void ResetTaps( void );
private:
  double Two_Mu;
};

#endif
