//
//  File = prog_22a.cpp
//
#include <stdlib.h> 
#include <iostream.h> 
#include <fstream.h>
#include <math.h>
#include "misdefs.h"
#include "complex.h"
#include "sig_type.h"
#include "ar_proc.h"
#include "ar_src.h"
#include "ar_est.h"
   

#ifdef _DEBUG
 ofstream DebugFile("prog_22a.bug", ios::out);
#endif

main()
{
  int seq_len, i;
  double true_ar_drv_var;
  long noise_seed=11123313;
  type_of_sig_vals_T *true_a_coeffs;
  type_of_sig_vals_T *sig_2;
  int true_ar_order, est_ar_order;
  
  cout << "Demo of programs for AR models" << endl;
  cout << "==============================\n" << endl;
  cout << "true AR order ?" << endl;
  cin >> true_ar_order;
  true_a_coeffs = new type_of_sig_vals_T[true_ar_order+1];
  for(i=1; i<=true_ar_order; i++)
    {
    cout << "enter value for a[" << i << "]" << endl;
    cin >> true_a_coeffs[i];
    }
  true_a_coeffs[0] = 1.0;

  cout << "enter variance for driving noise process" << endl;
  cin >> true_ar_drv_var;

  cout << "estimated AR order ?" << endl;
  cin >> est_ar_order;

  for(;;)
    {
    cout << "length of signal seq for correl. estim ?" << endl;
    cin >> seq_len;
    if(seq_len >= est_ar_order) break;
      cout << "seq length must be .GE. estimated AR order" << endl;
    }

  //- - - - - - - - - - - - - - - - - - - - - - - - -
  // the following instance of ArSource creates the
  // test sequence that will be used for estimating
  // the AR parameters

  
  ArProcess<type_of_sig_vals_T>* ar_src = 
              new ArSource<type_of_sig_vals_T>( 
                                      true_ar_order,
                                      true_a_coeffs,
                                      true_ar_drv_var );
  sig_2 = ar_src->OutputSequence( noise_seed, seq_len );
  
  //- - - - - - - - - - - - - - - - - - - - - - - - - -
  //  estimate the AR parameters from the random 
  //  sequence contained in the vector sig_2

  ArProcess<type_of_sig_vals_T>* est_ar_src = 
                 new ArEstimate<type_of_sig_vals_T>
                                        ( est_ar_order,
                                          sig_2,
                                          seq_len);
  est_ar_src->DumpParameters(cout);
  //- - - - - - - - - - - - - - - - - - - - - - - - - -

  delete ar_src;
  delete[] sig_2;
  return 0;
}  
