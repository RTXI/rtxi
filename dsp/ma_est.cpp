//
//  File = ma_est.cpp
//

#include <fstream>
#include <stdlib.h>
#include <iostream>

#include "ma_est.h"
#include "gausrand.h"
#include "sig_type.h"
#include "yulewalk.h"

#ifdef _DEBUG
extern std::ofstream DebugFile;
#endif

using namespace std;

//========================================================
//  MaEstimate - subclass of MaProcess for the case where
//  the MA coefficients must be estimated from observed data

template <class T>
MaEstimate<T>::MaEstimate(int est_ma_order, int durbin_ar_order, T* sig_seq,
                          int seq_len)
  : MaProcess<T>()
{
  int i, err_stat;
  T* a_coeffs;
  YuleWalker<T>* yw_ptr;

  this->Ma_Order = est_ma_order;
  this->Noise_Seed = 31415927; // arbitrary default

  this->Old_Input = new T[est_ma_order + 1];
  for (i = 0; i <= est_ma_order; i++)
    this->Old_Input[i] = 0.0;

  //---------------------------------------------------------
  //  Fit high-order AR model to the data

  a_coeffs = new T[durbin_ar_order + 1];

  yw_ptr = new YuleWalker<T>(sig_seq, seq_len, durbin_ar_order, a_coeffs,
                             &(this->Drv_Noise_Var), &err_stat);

  delete yw_ptr;

  //----------------------------------------------------------
  //  Use high-order AR coefficients in place of data
  //  to fit desired order MA model

  double dummy_var;
  this->B_Coeffs = new T[est_ma_order + 1];

  yw_ptr = new YuleWalker<T>(a_coeffs, durbin_ar_order + 1, est_ma_order,
                             this->B_Coeffs, &dummy_var, &err_stat);

  delete yw_ptr;
  delete[] a_coeffs;
  return;
}

//------------------------------------
//  Explicit instantiations
template class MaEstimate<type_of_sig_vals_T>;
// template class MaEstimate<complex>;
