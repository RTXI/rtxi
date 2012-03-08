//
//  File = lms_filt.cpp
//

#include <stdlib.h>
#include <fstream>
#include "lms_filt.h"
#include "adap_fir.h"

#ifdef _DEBUG
extern std::ofstream DebugFile;
#endif

LmsFilter::LmsFilter( int num_taps,
                      double *coeff,
                      double mu,
                      logical quan_enab,
                      long coeff_quan_factor,
                      long input_quan_factor,
                      int tap_for_trans,
                      int secondary_tap,
                      int transient_len)
         :AdaptiveFir( num_taps,
                       coeff,
                       quan_enab,
                       coeff_quan_factor,
                       input_quan_factor,
                       tap_for_trans,
                       secondary_tap,
                       transient_len)
{
 Two_Mu = 2.0*mu;
 #ifdef _DEBUG
 DebugFile << "In LmsFilter constructor" << std::endl;
 #endif
 return;
}

void LmsFilter::ResetTaps( void )
{
  int tap_idx;

  for(tap_idx=0; tap_idx<Num_Taps; tap_idx++)
    {
    Unquan_Coeff[tap_idx] = 0.0;
    Unquan_In_Buf[tap_idx] = 0.0;
    }
  Update_Count = 0;
  Trial_Count++;
}
double LmsFilter::UpdateTaps( double true_samp,
                              double estim_samp,
                              logical trans_save_enabled )
{
  double err_samp;
  int tap_idx, read_idx;
  read_idx = Write_Indx - 1;
  if(read_idx < 0) read_idx = Num_Taps - 1;
  err_samp = true_samp - estim_samp;
  for(tap_idx=0; tap_idx<Num_Taps; tap_idx++)
    {
    Unquan_Coeff[tap_idx] += 
                 Two_Mu * err_samp * Unquan_In_Buf[read_idx];
    if( (tap_idx == Tap_For_Trans) &&
        (Update_Count < Transient_Len) )
      {
      Tally_For_Avg[Update_Count] += Unquan_Coeff[tap_idx];

      if(trans_save_enabled)
      Sample_Transient[Update_Count] = Unquan_Coeff[tap_idx];
      }
    if( (tap_idx == Secondary_Tap) &&
        (Update_Count < Transient_Len) )
      {
      Tally_For_Avg_2[Update_Count] += Unquan_Coeff[tap_idx];
      if(trans_save_enabled)
      Sample_Trans_2[Update_Count] = Unquan_Coeff[tap_idx];
      }
    read_idx--;
    if(read_idx < 0) read_idx = Num_Taps - 1;
    }
  Update_Count++;
  return(err_samp);
}


