//
//  File = prog_22b.cpp
//
#include <stdlib.h> 
#include <iostream.h> 
#include <fstream.h>
#include <math.h>
#include "misdefs.h"
#include "complex.h"
#include "matrix.h"
#include "ma_est.h"
#include "ma_src.h"
#include "yulewalk.h"
#include "gausrand.h"
#include "sig_type.h"
   
#ifdef _DEBUG
 ofstream DebugFile("prog_22b.dbg", ios::out);
#endif

main()
{
  int seq_len, i;
  double true_ma_drv_var;
  long noise_seed=11123313;
  type_of_sig_vals_T *true_b_coeffs;
  type_of_sig_vals_T *sig_2;
  int true_ma_order, est_ma_order;
  int durbin_ar_order;
  
  ofstream out_file_1("p22b_of1.txt", ios::out);

  cout << "Demo of programs for MA models" << endl;
  cout << "==============================\n" << endl;
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
  cin >> true_ma_drv_var;

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
  // the following instance of MaSource creates the
  // test sequence that will be used for estimating
  // the MA parameters

  MaSource < type_of_sig_vals_T >* ma_model = 
        new MaSource< type_of_sig_vals_T >( 
                                     true_ma_order,
                                     true_b_coeffs,
                                     true_ma_drv_var );
  sig_2 = ma_model->OutputSequence( noise_seed, seq_len );

  //- - - - - - - - - - - - - - - - - - - - - - - - - -
  //  The following instance of MaModel estimates the
  //  MA parameters from the random sequence contained
  //  in the vector sig_2

  MaEstimate<type_of_sig_vals_T>* est_ma_model = 
                    new MaEstimate<type_of_sig_vals_T>( 
                                             est_ma_order,
                                             durbin_ar_order,
                                             sig_2,
                                             seq_len);
  est_ma_model->DumpParameters(cout);
  //- - - - - - - - - - - - - - - - - - - - - - - - - -

  delete ma_model;
  delete[] sig_2;
  out_file_1.close();
  return 0;
}  
