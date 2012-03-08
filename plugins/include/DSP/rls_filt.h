//
//  File = rls_filt.h
//

#ifndef _RLS_FILT_H_
#define _RLS_FILT_H_

#include "adap_fir.h"
#include "matrix_T.h"
//#include "typedefs.h"

class RlsFilter : public AdaptiveFir
{
public:
  RlsFilter( int num_taps,
             double *coeff,
             double delta,
             double lambda,
             logical quan_enab,
             long coeff_quan_factor,
             long input_quan_factor,
             int tap_for_trans,
             int secondary_tap,
             int transient_len);
  double UpdateTaps( double true_samp,
                     double estim_samp,
                     logical trans_save_enab );
  void ResetTaps( void );
private:
  double Lambda;
  double Delta;
  colvec<double> *Cv_Work;
  rowvec<double> *Rv_Work;
  colvec<double> *K_Vec;
  colvec<double> *U_Vec;
  matrix<double> *P_Mtx;
};

#endif
