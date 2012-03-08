//
//  File = ex26_04.cpp
//
#include <stdlib.h> 
#include <iostream.h> 
#include <fstream.h>
#include <math.h>
#include "misdefs.h"
#include "complex.h"
#include "matrix.h"
#include "ar_proc.h"
#include "ar_src.h"
#include "ar_est.h"
#include "arspec.h"
#include "burg.h"
#include "gausrand.h"
   

#ifdef _DEBUG
 ofstream DebugFile("ch_26.dbg", ios::out);
#endif

main()
{
  int seq_len, i;
  int int_input;
  double true_ar_drv_var;
  long default_data_seed=11123313;
  long default_noise_seed=695569;
  long seed;
  double *true_a_coeffs;
  double *sig_2;
  double noise_var=0.0;
  int true_ar_order, est_ar_order;
  double samp_intvl;
  double *est_a_coeffs;
  double est_drv_var;
  logical db_plot_enab;
  ArSpectrum<double> *ar_spectrum;
  

  cout << "Program for Example 26.4" << endl;
  cout << "========================\n" << endl;

  cout << "enable plotting in decibels?\n"
       << " 0 = No\n" << " 1 = Yes" << endl;
  cin >> int_input;
  db_plot_enab = (logical)int_input;

  cout << "length of signal seq for correl. estim ?" << endl;
  cin >> seq_len;

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

  cout << "\nenter seed for driving noise process\n"
       << "enter 0 to keep default of " 
       << default_data_seed << endl;
  cin >> seed;
  if (seed == 0) seed = default_data_seed;

  ar_src = new ArSource<double>(true_ar_order,
                                true_a_coeffs,
                                true_ar_drv_var);
  cout << "Now preparing ideal AR spectrum" << endl;
  ar_spectrum = new ArSpectrum<double>( true_ar_order,
                                        true_a_coeffs,
                                        1.0,
                                        true_ar_drv_var );
  ar_spectrum->DumpSpectrum( "i_arspec.txt\0",
                             db_plot_enab);
  cout << "ideal spectrum written to file i_arspec.txt" << endl;
  sig_2 = ar_src->OutputSequence( seed, seq_len );

  ofstream test_sig_file("test_sig.txt", ios::out);
  cout << "test signal written to file 'test_sig.txt'" << endl;
  for(i=0; i<seq_len; i++)
        test_sig_file << i << ", " << sig_2[i] << endl;
  test_sig_file.close();

  //- - - - - - - - - - - - - - - - - - - - - - - - - -
  //  Use Burg method to estimate AR spectrum

  cout << "estimated AR order ?" << endl;
  cin >> est_ar_order;

  BurgMethod<double> *spect_est;
  spect_est = new BurgMethod<double>( est_ar_order,
                                      sig_2,
                                      seq_len);
  est_a_coeffs = new double[est_ar_order+1];
  spect_est->GetParameters(&est_ar_order,est_a_coeffs);
  est_drv_var = spect_est->GetDrivingVariance();
  est_a_coeffs[0] = 1.0;

  cout << "Now preparing Burg-method AR spectrum estimate" << endl;
  ar_spectrum = new ArSpectrum<double>( est_ar_order,
                                        est_a_coeffs,
                                        samp_intvl,
                                        est_drv_var );
  ar_spectrum->DumpSpectrum( "ar_spec.txt\0",
                             db_plot_enab );

  return 0;
}  
