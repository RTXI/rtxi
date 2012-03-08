//
//  File = ex24_09.cpp
//
#include <stdlib.h> 
#include <iostream.h> 
#include <fstream.h>
#include <math.h>
#include <time.h>
#include "misdefs.h"
#include "complex.h"
#include "adap_fir.h"
#include "lms_filt.h"
#include "gausrand.h"
#include "learncur.h"
   

#ifdef _DEBUG
 ofstream DebugFile("adaptive.bug", ios::out);
 ofstream SpecFile("special.bug", ios::out);
#endif

main()
{
  int seq_len, i, samp_idx, num_trials;
  int output_decim_factor;
  double true_ar_drv_var;
  long noise_seed=11123313;
  double orig_samp, estim_samp, err_samp;
  double mu;
  double norm_sine_freq;
  double arg_mult, noise_var, noise_std_dev;
  int num_taps, tap_to_watch, depend_tap;

  AdaptiveFir *adapt_filt;
  
  ofstream out_file_1("ex24_09.txt", ios::out);

  cout << "Program for Example 24.9" << endl;
  cout << "=====================================\n" << endl;

  //- - - - - - - - - - - - - - - - - - - - - - - - -
  // create the test sequence that will be used as 
  // input to the adaptive filter

  cout << "sine freq as fraction of sample rate" << endl;
  cin >> norm_sine_freq;
  arg_mult = norm_sine_freq* TWO_PI;
  cout << "variance for additive noise process" << endl;
  cin >> noise_var;
  noise_std_dev = sqrt(noise_var);

  cout << "number of taps in adaptive filter ?" << endl;
  cin >> num_taps;

  if(num_taps > 1)
    {
    cout << "monitor transient of which tap ? (0,...,"
         << num_taps-1 << ")" << endl;
    cin >> tap_to_watch;
    cout << "dependent tap for trajectory plots?" << endl;
    cin >> depend_tap;
    }
  else
    {
    tap_to_watch = 0;
    depend_tap = -1;
    }

  cout << "length of signal seq ?" << endl;
  cin >> seq_len;

  cout << "output decimation factor for transients" << endl;
  cin >> output_decim_factor;

  double min_trajec_dist;
  cout << "min pt-pt distance for trajectory plot" << endl;
  cin >> min_trajec_dist;

  cout << "numb trials in average" << endl;
  cin >> num_trials;

  logical trans_save_enab;
  LOGICAL_T save_this_trial;
  int trial_to_save;
  cout << "save transient for which trial?" << endl;
  cin >> trial_to_save;
  if(trial_to_save >= num_trials) trial_to_save = num_trials-1;

  //- - - - - - - - - - - - - - - - - - - - - - - - - -
  //  instantiate the adaptive filter

  logical quan_enab = 0;
  long coeff_quan_factor = 0;
  long input_quan_factor = 0;
  double* coeff = new double[num_taps];
  for(i=0; i<num_taps; i++) coeff[i] = 0.0;

  int total_seq_len;
  total_seq_len = seq_len;

  cout << "adaptation gain (mu) ?" << endl;
  cin >> mu;
  adapt_filt = new LmsFilter( num_taps,
                              coeff,
                              mu,
                              quan_enab,
                              coeff_quan_factor,
                              input_quan_factor,
                              tap_to_watch,
                              depend_tap,
                              seq_len);
  //- - - - - - - - - - - - - - - - - - - - - - - - - -
  // initialize learning curve

  double min_mse = true_ar_drv_var;
  LearningCurve* learn_curve = new LearningCurve(
                                     seq_len,
                                     num_trials,
                                     min_mse );
  //- - - - - - - - - - - - - - - - - - - - - - - - - -
  //  loop over multiple experiments across ensemble

  double delayed_samp, desired_samp;
  int trial_idx;

  for( trial_idx=0; trial_idx<num_trials; trial_idx++)
  {
    trans_save_enab = FALSE;
    save_this_trial = _FALSE;
    if(trial_idx == trial_to_save) 
      {
      trans_save_enab = TRUE;
      save_this_trial = _TRUE;
      }
    adapt_filt->ResetTaps();
    //- - - - - - - - - - - - - - - - - - - - - - - - - -
    //  begin sample-by-sample loop within single trial

    delayed_samp = 0.0;

    for( samp_idx=0; samp_idx<total_seq_len; samp_idx++)
    {
      orig_samp = sin(arg_mult * samp_idx);
      orig_samp += noise_std_dev * GaussRandom(&noise_seed);
      estim_samp = adapt_filt->ProcessSample(orig_samp);
      desired_samp = cos(arg_mult * samp_idx);
      #ifdef _DEBUG
      if(trial_idx == trial_to_save)
         SpecFile << samp_idx << ", " 
                  << orig_samp << ", "
                  << desired_samp << endl;
      #endif
      err_samp = adapt_filt->UpdateTaps( desired_samp, 
                                         estim_samp, 
                                         trans_save_enab);
      delayed_samp = sqrt(1.0 - orig_samp*orig_samp);

    } // end of loop over samp_idx
  } // end of loop over trial_idx

  double* estim_taps = new double[num_taps+1];
  adapt_filt->GetTaps(estim_taps);
  for(int jj=num_taps; jj>0; jj--)
    {
    estim_taps[jj] = -estim_taps[jj-1];
    cout << "tap[" << jj << "] = " << estim_taps[jj] << endl;
    }
  adapt_filt->DumpTransient(output_decim_factor);
  adapt_filt->DumpAvgTransient(output_decim_factor);
  adapt_filt->DumpAvgTrajectory( min_trajec_dist);
  adapt_filt->DumpTrajectory( min_trajec_dist);

  out_file_1.close();
  return 0;
}  
