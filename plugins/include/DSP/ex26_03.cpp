//
//  File = ex26_03.cpp
//
#include <stdlib.h> 
#include <iostream.h> 
#include <fstream.h>
#include <math.h>
#include "misdefs.h"
#include "adaptype.h"
#include "complex.h"
#include "matrix.h"
#include "ar_est.h"
#include "arspec.h"
#include "cpfsk.h"
#include "fsk_spec.h"
#include "burg.h"
#include "pause.h"
#include "gausrand.h"
   

#ifdef _DEBUG
 ofstream DebugFile("ex26_03.dbg", ios::out);
#endif

main()
{
  int seq_len, i;
  int int_input;
  long default_data_seed=11123313;
  long default_noise_seed=695569;
  long seed;
  double *sig_2;
  int est_ar_order;
  double carrier_freq, fsk_freq_dev;
  double samp_intvl;
  double *est_a_coeffs;
  double est_drv_var;
  int fsk_big_m;
  double fsk_symb_dur;
  int fsk_samp_per_symb;
  int samp_per_seg;
  logical db_plot_enab;
  SignalSource *signal_source;
  ArSpectrum<double> *ar_spectrum;
  CpfskSpectrum *ref_spectrum;
  

  cout << "Program for Example 26.3" << endl;
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

  sig_2 = new double[seq_len];

  cout << "CPFSK alphabet size?" << endl;
  cin >> fsk_big_m;

  cout << "carrier frequency for CPFSK ?" << endl;
  cin >> carrier_freq;

  cout << "sampling interval?" << endl;
  cin >> samp_intvl;

  cout << "samples per FSK symbol?" << endl;
  cin >> fsk_samp_per_symb;
  fsk_symb_dur = fsk_samp_per_symb * samp_intvl;

  cout << "frequency deviation for CPFSK ?" << endl;
  cin >> fsk_freq_dev;

  cout << "\nenter seed for random CPFSK symbol seq\n"
       << "enter 0 to keep default of " 
       << default_data_seed << endl;
  cin >> seed;
  if (seed == 0) seed = default_data_seed;

  samp_per_seg=seq_len;

  ref_spectrum = new CpfskSpectrum( fsk_big_m,
                                    fsk_freq_dev,
                                    fsk_symb_dur);
  signal_source = new CpfskSource( samp_intvl,
                                   fsk_freq_dev,
                                   fsk_samp_per_symb,
                                   seed,
                                   samp_per_seg,
                                   0);

  signal_source->GetNextSegment(sig_2, seq_len);
  ofstream test_sig_file("test_sig.txt", ios::out);
  cout << "test signal written to file 'test_sig.txt'" << endl;
  for(i=0; i<seq_len; i++)
        test_sig_file << i << ", " << sig_2[i] << endl;
  test_sig_file.close();

  //- - - - - - - - - - - - - - - - - - - - - - - - - -
  //  estimate the AR parameters from the random 
  //  sequence contained in the vector sig_2

  cout << "estimated AR order ?" << endl;
  cin >> est_ar_order;

  ArProcess<double> *est_ar_src;
  est_ar_src = new ArEstimate<double>( est_ar_order,
                                       sig_2,
                                       seq_len);
  est_ar_src->DumpParameters(cout);

  est_a_coeffs = new double[est_ar_order+1];
  est_ar_src->GetParameters(&est_ar_order,est_a_coeffs);
  est_drv_var = est_ar_src->GetDrivingVariance();
  cout << "est_a_coeffs[0] = " << est_a_coeffs[0] << endl;
  cout << "Now preparing Yule-Walker AR spectrum estimate" 
       << endl;
  ar_spectrum = new ArSpectrum<double>( est_ar_order,
                                        est_a_coeffs,
                                        samp_intvl,
                                        est_drv_var );
  ar_spectrum->DumpSpectrum( "ar_spec.txt\0",
                             ref_spectrum,
                             db_plot_enab );

  delete[] sig_2;
  return 0;
}  
