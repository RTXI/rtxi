//
//  File = ma_src.cpp
//

#include <fstream>
#include <stdlib.h>
#include <iostream>

#include "ma_src.h"
#include "sig_type.h" //selects signal type for template
                      // instantiation

#ifdef _DEBUG
extern std::ofstream DebugFile;
#endif

//======================================================
//  MaSource - subclass of MaProcess for the case where
//             the AR coefficients are known or assumed
//             and are provided as input parameters

template <class T>
MaSource<T>::MaSource(int ma_order, T* b_coeffs, double drv_noise_var)
  : MaProcess<T>()
{
  int i;
  this->Ma_Order = ma_order;
  this->Noise_Seed = 31415927; // arbitrary default
  this->Drv_Noise_Var = drv_noise_var;

  this->B_Coeffs = new T[ma_order + 1];
  for (i = 0; i <= ma_order; i++)
    this->B_Coeffs[i] = b_coeffs[i];

  this->Old_Input = new T[ma_order + 1];
  for (i = 0; i <= ma_order; i++)
    this->Old_Input[i] = 0.0;
}

template <class T>
MaSource<T>::~MaSource(void)
{
}

//-----------------------------------------------
//  Explicit instantiations
// template class MaSource<double>;
template class MaSource<type_of_sig_vals_T>;
