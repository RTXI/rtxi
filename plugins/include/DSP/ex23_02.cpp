//
//  File = lin_pred.cpp
//
#include <stdlib.h> 
#include <iostream.h> 
#include <iomanip.h>
#include <fstream.h>
#include <math.h>
#include "misdefs.h"
#include "pause.h"
#include "complex.h"
#include "ar_est.h"
#include "laguerre.h"
#include "polefunc.h"
#include "bilinear.h"
#include "unq_iir.h"
   

#ifdef _DEBUG
 ofstream DebugFile("lin_pred.bug", ios::out);
#endif

main()
{
  int seq_len, i;
  long noise_seed=11123313;
  double *sig_2;
  int est_ar_order;
  double out_samp, samp_intvl;
  complex *a_coeff=NULL;
  double *real_a_coeff=NULL, *real_b_coeff=NULL, *denom_coeff=NULL;
  double single_numer_coeff[1];
  complex *cp_roots;
  int ii;
  FilterTransFunc* anlg_filt_func;
  IirFilterDesign *dig_filt_dsgn;
  FilterImplementation *filt_implem;
  FilterImplementation *filt_implem_2;
  CmplxPolynomial *char_poly, root_binomial;
  
  ofstream out_file_1("LP_of1.txt", ios::out);

  cout << "Demo of programs for linear prediction" << endl;
  cout << "======================================\n" << endl;

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

  anlg_filt_func = new AllPoleTransFunc(cin, cout);
  anlg_filt_func->LowpassDenorm(1.0);

  anlg_filt_func->FrequencyPrewarp( samp_intvl );
  dig_filt_dsgn = BilinearTransf( anlg_filt_func,
                                  samp_intvl);
  #ifdef _DEBUG
  dig_filt_dsgn->DumpCoefficients( &DebugFile);
  #endif

  delete anlg_filt_func;

  //--------------------------------------------------------
  //  Check to see if test signal generator will be stable

  int cp_order = dig_filt_dsgn->GetNumDenomCoeffs();
  cout << "cp_order = " << cp_order << endl;
  complex* cp_coeff = new complex[cp_order+1];
  denom_coeff = dig_filt_dsgn->GetDenomCoefficients();

  //--pppppppppppppppppppppppppppppppppppppppppppppppppp
  // patch to allow arbitrary denom coeffs that did not
  // come from the bilinear transformation
  int mod_flag;
  cout << "want chance to modify IIR coeffs?" << endl;
  cout << "( 0=NO, 1=YES )" << endl;
  cin >> mod_flag;
  if(mod_flag >0)
    {
    double d_val;
    cout << setprecision(10);
    for(ii=1; ii<=cp_order; ii++)
      {
      d_val = denom_coeff[ii];
      cout << "denom_coeff[" << ii << "] = " 
           << d_val << endl;
      cout << "enter modified value " << endl;
      cin >> d_val;
      denom_coeff[ii] = d_val;
      }
    cout << setprecision(6) << flush;
    dig_filt_dsgn->SetDenomCoefficients(cp_order, denom_coeff);
    }
  // end of patch
  //--pppppppppppppppppppppppppppppppppppppppppppppppppp
  cp_coeff[cp_order] = complex(1.0,0.0);
  for( ii=0; ii<cp_order; ii++)
    cp_coeff[ii] = -denom_coeff[cp_order-ii];

  char_poly = new CmplxPolynomial( cp_coeff, cp_order );
  pause();
  cp_roots = char_poly->GetRoots();

  for( ii=0; ii<est_ar_order; ii++)
    {
    if( cabs(cp_roots[ii])<1.0) continue;
      cout << "generator root " << ii << " (val=" 
           << cp_roots[ii] << ")\n is outside the unit circle" 
           << endl;
      char_poly->ReflectRoot(ii);
      cout << "reflecting this root" << endl;
      pause();
    }
  delete [] cp_coeff;
  delete char_poly;

  //--------------------------------------------------------
  //  Generate segment of test signal

  single_numer_coeff[0] = 1.0;     
  filt_implem = new UnquantDirectFormIir(
                 1,
                 dig_filt_dsgn->GetNumDenomCoeffs(),
                 single_numer_coeff,
                 dig_filt_dsgn->GetDenomCoefficients() );
                
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
  //  The following instance of ArModel estimates the
  //  linear predictor coefficients for the signal 
  //  contained in the vector sig_2

  ArEstimate<double>* lp_estimator = 
                     new ArEstimate<double>( est_ar_order,
                                             sig_2,
                                             seq_len);
  lp_estimator->DumpParameters(cout);
  #ifdef _DEBUG
  lp_estimator->DumpParameters(DebugFile);
  #endif
  pause();
  real_a_coeff = new double[est_ar_order+1];
  if(real_a_coeff == 0)
    {
    cout << "failed to allocate real_a_coeff" << endl;
    exit(70);
    }
  real_b_coeff = new double[1];
  real_b_coeff[0] = 1.0;

  lp_estimator->GetParameters(&est_ar_order, real_a_coeff);
  cout << "est_ar_order = " << est_ar_order << endl;
  real_a_coeff[0] = 1.0;
  for(i=1; i<=est_ar_order; i++)
    {
    real_a_coeff[i] = -real_a_coeff[i];
    cout << "real a[" << i << "] = " << real_a_coeff[i] << endl;
    }
  //-----------------------------------------------
  //  check for stability by finding roots of the
  //  characteristic polynomial.

  cp_coeff = new complex[est_ar_order+1];
  cp_coeff[est_ar_order] = complex(1.0,0.0);
  for( ii=0; ii<est_ar_order; ii++)
    cp_coeff[ii] = complex(-real_a_coeff[est_ar_order-ii],0.0);
  char_poly = new CmplxPolynomial( cp_coeff, est_ar_order);
  pause();
  cp_roots = char_poly->GetRoots();

  for( ii=0; ii<est_ar_order; ii++)
    {
    if( cabs(cp_roots[ii])<1.0) continue;
      cout << "root " << ii << " (val=" << cp_roots[ii]
           << ") is outside the unit circle" << endl;
      exit(66);
    }

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
  delete [] a_coeff;
  delete [] real_a_coeff;
  delete [] real_b_coeff;
  delete lp_estimator;
  delete char_poly;
  delete[] sig_2;
  out_file_1.close();
  return 0;
}  
