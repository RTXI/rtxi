//
//  File = main_22c.cpp
//
#include <stdlib.h> 
#include <iostream.h> 
#include <fstream.h>
#include <math.h>
#include "misdefs.h"
#include "complex.h"
#include "sig_type.h"
#include "matrix.h"
#include "arma_src.h"
#include "arma_est.h"
#include "ar_proc.h"
#include "yulewalk.h"
#include "gausrand.h"   

#ifdef _DEBUG
 ofstream DebugFile("prog_29c.bug", ios::out);
#endif

main()
{
  int seq_len, i;
  double true_arma_drv_var;
  long noise_seed=11123313;
  type_of_sig_vals_T *true_a_coeffs;
  type_of_sig_vals_T *true_b_coeffs;
  type_of_sig_vals_T *sig_2;
  int true_ar_order, est_ar_order;
  int true_ma_order, est_ma_order;
  int durbin_ar_order;
  
  ofstream out_file_1("p29c_of1.txt", ios::out);

  cout << "Demo of programs for ARMA models" << endl;
  cout << "==============================\n" << endl;
  cout << "true AR order ?" << endl;
  cin >> true_ar_order;
  true_a_coeffs = new type_of_sig_vals_T[true_ar_order+1];
  for(i=1; i<=true_ar_order; i++)
    {
    cout << "enter value for a[" << i << "]" << endl;
    cin >> true_a_coeffs[i];
    cout << "value as read is " << true_a_coeffs[i] << endl;
    }
  true_a_coeffs[0] = 1.0;

  cout << "true MA order ?" << endl;
  cin >> true_ma_order;
  true_b_coeffs = new type_of_sig_vals_T[true_ma_order+1];
  for(i=1; i<=true_ma_order; i++)
    {
    cout << "enter value for b[" << i << "]" << endl;
    cin >> true_b_coeffs[i];
    }
  true_b_coeffs[0] = 1.0;

  cout << "enter variance for driving noise process" << endl;
  cin >> true_arma_drv_var;

  cout << "estimated AR order ?" << endl;
  cin >> est_ar_order;

  cout << "estimated MA order ?" << endl;
  cin >> est_ma_order;

  cout << "AR order for Durbin algorithm ?" << endl;
  cin >> durbin_ar_order;

  for(;;)
    {
    cout << "length of signal seq for correl. estim ?" << endl;
    cin >> seq_len;
    if(seq_len >= est_ma_order) break;
      cout << "seq length must be .GE. estimated MA order" << endl;
    }

  //- - - - - - - - - - - - - - - - - - - - - - - - -
  // the following instance of ArmaModel creates the
  // test sequence that will be used for estimating
  // the ARMA parameters

  ArmaSource<type_of_sig_vals_T>* arma_model = 
                        new ArmaSource<type_of_sig_vals_T>
                                         ( true_ar_order,
                                           true_a_coeffs,
                                           true_ma_order,
                                           true_b_coeffs,
                                           true_arma_drv_var );

  sig_2 = arma_model->OutputSequence( noise_seed, seq_len );

  //- - - - - - - - - - - - - - - - - - - - - - - - - -
  //  The following instance of ArmaModel estimates the
  //  ARMA parameters from the random sequence contained
  //  in the vector sig_2

  ArmaEstimate<type_of_sig_vals_T>* est_arma_model = 
                     new ArmaEstimate<type_of_sig_vals_T>
                                           ( est_ar_order,
                                             est_ma_order,
                                             durbin_ar_order,
                                             sig_2,
                                             seq_len);
  est_arma_model->DumpParameters(cout);
  //- - - - - - - - - - - - - - - - - - - - - - - - - -

  delete arma_model;
  delete est_arma_model;
  //delete est_ar_model;
  delete[] sig_2;
  out_file_1.close();
  return 0;
}  
