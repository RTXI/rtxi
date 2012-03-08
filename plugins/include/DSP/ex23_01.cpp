//
//  File = ex23_01.cpp
//
#include <stdlib.h> 
#include <iostream.h> 
#include <iomanip.h>
#include <fstream.h>
#include <math.h>
#include "misdefs.h"
#include "complex.h"
#include "ar_est.h"
#include "unq_iir.h"
   

#ifdef _DEBUG
 ofstream DebugFile("lin_pred.bug", ios::out);
#endif

main()
{
  int seq_len, i;
  long noise_seed=11123313;
  double *sig_2;
  int est_ar_order, true_ar_order;
  double out_samp, samp_intvl;
  complex *a_coeff=NULL;
  double *real_a_coeff=NULL, *real_b_coeff=NULL, *denom_coeff=NULL;
  double single_numer_coeff[1];
  int ii;
  FilterImplementation *filt_implem;
  FilterImplementation *filt_implem_2;
  
  ofstream out_file_1("LP_of1.txt", ios::out);

  cout << "Demo of programs for linear prediction" << endl;
  cout << "======================================\n" << endl;

  cout << "true AR order ?" << endl;
  cin >> true_ar_order;
  denom_coeff = new double[true_ar_order+1];

  cout << "estimated AR order ?" << endl;
  cin >> est_ar_order;

  for(;;)
    {
    cout << "length of signal seq for correl. estim ?" << endl;
    cin >> seq_len;
    if(seq_len >= est_ar_order) break;
      cout << "seq length must be .GE. estimated AR order" << endl;
    }

  //- - - - - - - - - - - - - - - - - - - - - - - - - -
  //  determine parameters to create a test signal

  ofstream inp_seq_file("inp_seq.txt", ios::out);
  sig_2 = new double[seq_len];


  samp_intvl = 1.0;

  double d_val;
  cout << setprecision(10);
  for(ii=1; ii<=true_ar_order; ii++)
    {
    cout << "enter denom_coeff[" << ii << "] " << flush;
    cin >> d_val;
    denom_coeff[ii] = d_val;
    }
  cout << setprecision(6) << flush;

  //--------------------------------------------------------
  //  Generate sequence using given AR coefficients
  single_numer_coeff[0] = 1.0;     
  filt_implem = new UnquantDirectFormIir(
                 1,
                 true_ar_order,
                 single_numer_coeff,
                 denom_coeff );
                
  i=0;
  out_samp = filt_implem->ProcessSample(1.0/samp_intvl);
  sig_2[i] = out_samp;
  inp_seq_file << i*samp_intvl << ", " << out_samp << endl;

  for(i=1; i<seq_len; i++)
    {
    out_samp = filt_implem->ProcessSample(0.0);
    sig_2[i] = out_samp;
    inp_seq_file << i*samp_intvl << ", " << out_samp << endl;
    }
  inp_seq_file.close();
  delete filt_implem;

  //- - - - - - - - - - - - - - - - - - - - - - - - - -
  //  Estimate the linear predictor coefficients for 
  //  the signal contained in the vector sig_2

  ArEstimate<double>* lp_estimator = 
                     new ArEstimate<double>( est_ar_order,
                                             sig_2,
                                             seq_len);
  lp_estimator->DumpParameters(cout);
  #ifdef _DEBUG
  lp_estimator->DumpParameters(DebugFile);
  #endif

  real_a_coeff = new double[est_ar_order+1];
  real_b_coeff = new double[1];
  real_b_coeff[0] = 1.0;

  lp_estimator->GetParameters(&est_ar_order, real_a_coeff);
  real_a_coeff[0] = 1.0;
  for(i=1; i<=est_ar_order; i++)
    {
    real_a_coeff[i] = -real_a_coeff[i];
    }

  //----------------------------------------------------
  // Generate sequence using estimated AR coefficients

  ofstream out_seq_file("out_seq.txt", ios::out);
  filt_implem_2 = new UnquantDirectFormIir(
                      1,
                      est_ar_order,
                      real_b_coeff,
                      real_a_coeff );
  i=0;
  out_samp = filt_implem_2->ProcessSample(1.0/samp_intvl);
  out_seq_file << i*samp_intvl << ", " << out_samp << endl;

  for(i=1; i<seq_len; i++)
    {
    out_samp = filt_implem_2->ProcessSample(0.0);
    out_seq_file << i*samp_intvl << ", " << out_samp << endl;
    }

  out_seq_file.close();
  delete filt_implem_2;
  //- - - - - - - - - - - - - - - - - - - - - - - - - -
  delete [] real_a_coeff;
  delete [] real_b_coeff;
  delete lp_estimator;
  delete[] sig_2;
  out_file_1.close();
  return 0;
}  
