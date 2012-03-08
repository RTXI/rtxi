//
//  File = ex23_03.cpp
//
#include <stdlib.h> 
#include <iostream.h> 
#include <fstream.h>
#include <iomanip.h>
#include <math.h>
#include "misdefs.h"
#include "complex.h"
#include "cholesky.h"
#include "arsigsrc.h"
#include "covmeth.h"
#include "pause.h"
   

#ifdef _DEBUG
 ofstream DebugFile("ex23_03.bug", ios::out);
#endif

main()
{
  int seq_len;
  int err_stat;
  int est_order;
  complex *signal;
  double samp_intvl;
  
  ofstream out_file_1("e23_3_f1.txt", ios::out);

  cout << "Demo of program for Cholesky decomp" << endl;
  cout << "==============================\n" << endl;

  double epsilon = 1.0e-15;

  cout << "estimation order ?" << endl;
  cin >> est_order;

  for(;;)
    {
    cout << "length of signal seq for correl. estim ?" << endl;
    cin >> seq_len;
    if(seq_len >= est_order) break;
      cout << "seq length must be .GE. estimation order" << endl;
    }

  //- - - - - - - - - - - - - - - - - - - - - - - - - -
  //  Create an autoregressive test signal

  ofstream inp_seq_file("inp_seq.txt", ios::out);
  signal = new complex[seq_len];

  ArSignalSource( cin, cout, &samp_intvl, seq_len, signal);

  CovarMethCorrMtx corr_mat( signal, seq_len, est_order-1);

  for(int i=1; i<=4; i++)
    {
    for(int j=1; j<=4; j++)
      {
      cout << "corr_mat[" << i << "][" << j << "] = " 
           << (corr_mat[i][j]) << endl;
      }
    }
  pause();

  complex* bx = CovarMethRightHandVect( signal,
                                        seq_len,
                                        est_order-1);
  for(i=0; i<=4; i++)
    {
    cout << "rv[" << i << "] = " << bx[i] << endl;
    }

  err_stat = CholeskyDecomp( est_order,
                             &corr_mat,
                             bx,
                             epsilon );
  cout << "err_stat = " << err_stat << endl;
  bx[0] = complex(0.99999, 0.0);
  for(int indx=0; indx<=est_order; indx++)
    {
    cout << "bx[" << indx << "] = " << (bx[indx]) << endl;
    }

  out_file_1.close();
  return 0;
}  
