//
//  File = ex26_08.cpp
//


#include <stdlib.h> 
#include <iostream.h> 
#include <fstream.h>
#include <math.h>
#include "misdefs.h"
#include "complex.h"
#include "ar_proc.h"
#include "armaproc.h"
#include "arma_est.h"
#include "ar_src.h"
#include "armaspec.h"
#include "gausrand.h"
#include "fsk_spec.h"
   

#ifdef _DEBUG
 ofstream DebugFile("ex26_08.dbg", ios::out);
#endif

main()
{
  int seq_len, i;
  int int_input;
  double true_ar_drv_var;
  long default_data_seed=11123313;
  long default_noise_seed=695569;
  long seed, noise_seed;
  double *true_a_coeffs;
  double *sig_2;
  double noise_var, noise_std_dev;
  int true_ar_order, est_ar_order;
  double samp_intvl;
  double *est_a_coeffs, *est_b_coeffs;
  double est_drv_var;
  logical db_plot_enab;
  ArmaSpectrum<double> *arma_spectrum;
  CpfskSpectrum *ref_spectrum;
  

  cout << "Program for Example 26.8" << endl;
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

  cout << "\nenter variance for observation noise" << endl;
  cin >> noise_var;
  noise_std_dev = sqrt(noise_var);

  if( noise_var == 0.0)
    {noise_seed = default_noise_seed;}
  else
    {
    cout << "enter seed for observation noise\n"
         << "enter 0 to keep default of " 
         << default_noise_seed << endl;
    cin >> noise_seed;
    if (noise_seed == 0) noise_seed = default_noise_seed;
    }

  ar_src = new ArSource<double>(true_ar_order,
                                true_a_coeffs,
                                true_ar_drv_var);
  ref_spectrum = NULL;
  sig_2 = ar_src->OutputSequence( seed, seq_len );
  for (i=0; i<seq_len; i++)
    {
    sig_2[i] += noise_std_dev * GaussRandom(&noise_seed);
    }

  //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // Use modified Yule-Walker method to estimate ARMA spectrum
  ArmaEstimate<double> *est_arma_model;
  int est_ma_order;
  int durbin_ar_order;
  cout << "order for AR estimation?" << endl;
  cin >> est_ar_order;
  cout << "order for MA estimation?" << endl;
  cin >> est_ma_order;
  cout << "AR order for Durbin's method?" << endl;
  cin >> durbin_ar_order;
  est_arma_model = 
              new ArmaEstimate<double>( est_ar_order,
                                        est_ma_order,
                                        durbin_ar_order,
                                        sig_2,
                                        seq_len);
  est_a_coeffs = new double[est_ar_order+1];
  est_b_coeffs = new double[est_ma_order+1];
  est_arma_model->GetParameters( &est_ar_order,
                                 est_a_coeffs,
                                 &est_ma_order,
                                 est_b_coeffs);
  est_drv_var = est_arma_model->GetDrivingVariance();
  cout << "Now preparing modified Yule-Walker\n"
       << "ARMA spectral estimate" << endl;
  arma_spectrum = new ArmaSpectrum<double>( est_ar_order,
                                            est_a_coeffs,
                                            est_ma_order,
                                            est_b_coeffs,
                                            samp_intvl,
                                            est_drv_var );
  arma_spectrum->DumpSpectrum( "armaspec.txt\0",
                               ref_spectrum,
                               db_plot_enab );

  delete ar_src;
  delete[] sig_2;
  return 0;
}  
