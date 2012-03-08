//
//  File = adap_fir.h
//

#ifndef _ADAP_FIR_H_
#define _ADAP_FIR_H_

#include "dirform1.h"
#include "typedefs.h"

class AdaptiveFir : public DirectFormFir
{
public:
  AdaptiveFir( int num_taps,
               double *coeff,
               logical quan_enab,
               long coeff_quan_factor,
               long input_quan_factor,
               int tap_for_trans,
               int secondary_tap,
               int transient_len);
  void DumpAvgTransient( int decim_factor );
  void DumpTransient( int decim_factor );
  void DumpAvgTrajectory( double min_dist );
  void DumpTrajectory( double min_dist );
  void GetTaps(double *taps);
  virtual double UpdateTaps( double true_samp,
                             double estim_samp,
                             logical trans_save_enab )=0;
  virtual void ResetTaps( void )=0;

protected:
  int Update_Count;
  int Trial_Count;
  int Tap_For_Trans;
  int Secondary_Tap;
  int Transient_Len;
  double* Tally_For_Avg;
  double* Tally_For_Avg_2;
  double* Sample_Transient;
  double* Sample_Trans_2;
  std::ofstream* Tap_File;
};

#endif
