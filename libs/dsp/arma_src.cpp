//
//  File = armaproc.cpp
//

#include "arma_src.h"
#include "gausrand.h"
#include "mod_yuwa.h"
#include "yulewalk.h"
#include <fstream>
#include <stdlib.h>

//=======================================================
//  ArmaSource - subclass of ArmaProcess for case where
//               AR coefficients are known or assumed and
//               are provided as input parameters

template <class T>
ArmaSource<T>::ArmaSource(int ar_order, T* a_coeffs, int ma_order, T* b_coeffs,
                          double drv_noise_var)
  : ArmaProcess<T>()
{
  int i;
  this->Ar_Order = ar_order;
  this->Ma_Order = ma_order;
  this->Noise_Seed = 31415927; // arbitrary default
  this->Drv_Noise_Var = drv_noise_var;

  this->A_Coeffs = new T[ar_order + 1];
  for (i = 0; i <= ar_order; i++)
    this->A_Coeffs[i] = a_coeffs[i];

  this->B_Coeffs = new T[ma_order + 1];
  for (i = 0; i <= ma_order; i++)
    this->B_Coeffs[i] = b_coeffs[i];

  this->Old_Input = new T[ma_order + 1];
  for (i = 0; i <= ma_order; i++)
    this->Old_Input[i] = 0.0;
  this->Old_Output = new T[ar_order];
  for (i = 0; i < ar_order; i++)
    this->Old_Output[i] = 0.0;
}
//-------------------------------------
//  Explicit instantiations of template
template class ArmaSource<double>;
template class ArmaSource<complex>;
