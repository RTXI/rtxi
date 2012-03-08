//
//  File = ex26_05.cpp
//
#include <stdlib.h> 
#include <iostream.h> 
#include <fstream.h>
#include <math.h>
#include <time.h>
#include "misdefs.h"
#include "complex.h"
#include "adaptype.h"
#include "ar_proc.h"
#include "ar_src.h"
#include "arspec.h"
#include "adap_fir.h"
#include "rls_filt.h"
#include "gausrand.h"   

#ifdef _DEBUG
 ofstream DebugFile("adaptive.bug", ios::out);
#endif

main()
{
  int seq_len, i, samp_idx;
  double true_ar_drv_var;
  double samp_intvl;
  long noise_seed=11123313;
  long seed;
  double *true_a_coeffs;
  double orig_samp, estim_samp, err_samp;
  int true_ar_order;
  double lambda, delta;
  int num_taps, tap_to_watch, depend_tap;
  int int_input;
  logical db_plot_enab;
  ArSpectrum<double> *ar_spectrum;

  AdaptiveFir *adapt_filt;
  
  cout << "Program for adaptive spectrum estimation" << endl;
  cout << "========================================\n" << endl;

  cout << "enable plotting in decibels?\n"
       << " 0 = No\n" << " 1 = Yes" << endl;
  cin >> int_input;
  db_plot_enab = (logical)int_input;

  //- - - - - - - - - - - - - - - - - - - - - - - - -
  // create the test sequence that will be used as 
  // input to the adaptive filter
  ArProcess<double> *ar_src;

  cout << "true AR order ?" << endl;
  cin >> true_ar_order;
  samp_intvl = 1.0;
  true_a_coeffs = new double[true_ar_order+1];
  for(i=1; i<=true_ar_order; i++)
    {
    cout << "enter value for a[" << i << "]" << endl;
    cin >> true_a_coeffs[i];
    }
  true_a_coeffs[0] = 1.0;

  cout << "enter variance for driving noise process" << endl;
  cin >> true_ar_drv_var;

  cout << "seed for driving noise generator" << endl;
  cin >> seed;

  ar_src = new ArSource<double>(true_ar_order,
                                true_a_coeffs,
                                true_ar_drv_var);
  ar_spectrum = new ArSpectrum<double>( true_ar_order,
                                        true_a_coeffs,
                                        1.0,
                                        true_ar_drv_var );
  ar_spectrum->DumpSpectrum( "i_arspec.txt\0",
                             db_plot_enab);

  cout << "number of taps in RLS adaptive filter ?" << endl;
  cin >> num_taps;

  cout << "length of signal sequence used for adapt.?" << endl;
  cin >> seq_len;

  logical trans_save_enab;
  LOGICAL_T save_this_trial;
  int trial_to_save=0;

  //- - - - - - - - - - - - - - - - - - - - - - - - - -
  //  instantiate the adaptive filter

  logical quan_enab = 0;
  long coeff_quan_factor = 0;
  long input_quan_factor = 0;
  tap_to_watch = -1;
  depend_tap = -1;
  double* coeff = new double[num_taps];
  for(i=0; i<num_taps; i++) coeff[i] = 0.0;

  int total_seq_len;
  total_seq_len = seq_len;

  cout << "desired value of delta (0.01 typ.) ?" << endl;
  cin >> delta;
  cout << "desired value of lambda (0.95 typ.) ?" << endl;
  cin >> lambda;
  adapt_filt = new RlsFilter( num_taps,
                              coeff,
                              delta,
                              lambda,
                              quan_enab,
                              coeff_quan_factor,
                              input_quan_factor,
                              tap_to_watch,
                              depend_tap,
                              seq_len);
  //- - - - - - - - - - - - - - - - - - - - - - - - - -

  double delayed_samp, desired_samp;
  double ar_var;
  orig_samp = ar_src->NextSample(seed);


  trans_save_enab = FALSE;
  save_this_trial = _FALSE;
  adapt_filt->ResetTaps();
  //- - - - - - - - - -  - - - - - - - - - - -
  //  begin sample-by-sample loop

  delayed_samp = 0.0;

  for( samp_idx=0; samp_idx<total_seq_len; samp_idx++)
  {
    orig_samp = ar_src->NextSample();
    estim_samp = adapt_filt->ProcessSample(delayed_samp);
    desired_samp = orig_samp;

    err_samp = adapt_filt->UpdateTaps( desired_samp, 
                                       estim_samp, 
                                       trans_save_enab);
    delayed_samp = orig_samp;
  } // end of loop over samp_idx

  ar_var = ar_src->GetVariance();
  double* estim_taps = new double[num_taps+1];
  adapt_filt->GetTaps(estim_taps);
  for(int jj=num_taps; jj>0; jj--)
    {
    estim_taps[jj] = -estim_taps[jj-1];
    cout << "tap[" << jj << "] = " << estim_taps[jj] << endl;
    }
  estim_taps[0] = 1.0;
  ar_spectrum = new ArSpectrum<double>( num_taps,
                                        estim_taps,
                                        samp_intvl,
                                        ar_var);
  ar_spectrum->DumpSpectrum( "adapspec.txt\0",
                             db_plot_enab);
  cout << "AR estimated spectrum written to 'adapspec.txt'" << endl;
  return 0;
}  
