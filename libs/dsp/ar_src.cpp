//
//  File = ar_src.cpp
//

#include <fstream>
#include <stdlib.h>
#include "ar_src.h"
#include "sig_type.h"

//======================================================
//  ArSource - subclass of ArProcess for the case where
//             AR coefficients are known or assumed and
//             are provided as input parameters

template <class T>
ArSource<T>::ArSource(int ar_order, T* a_coeffs, double drv_noise_var)
  : ArProcess<T>()
{
  int i;
  this->Ar_Order = ar_order;
  this->Noise_Seed = 31415927; // arbitrary default
  this->Drv_Noise_Var = drv_noise_var;

  this->A_Coeffs = new T[ar_order + 1];
  for (i = 0; i <= ar_order; i++)
    this->A_Coeffs[i] = a_coeffs[i];

  this->Old_Output = new T[ar_order];
  for (i = 0; i < ar_order; i++)
    this->Old_Output[i] = 0.0;
}
template <class T>
ArSource<T>::~ArSource(void){};
//---------------------------
// Explicit instantiations
template class ArSource<type_of_sig_vals_T>;
