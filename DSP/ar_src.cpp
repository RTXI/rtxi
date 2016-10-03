//
//  File = ar_src.cpp
//

#include <stdlib.h>
#include <fstream>
#include "sig_type.h"
#include "ar_src.h"

//======================================================
//  ArSource - subclass of ArProcess for the case where 
//             AR coefficients are known or assumed and 
//             are provided as input parameters

template<class T>
ArSource<T>::ArSource( int ar_order,
                       T *a_coeffs,
                       double drv_noise_var )
            :ArProcess<T>()
{
  int i;
  Ar_Order = ar_order;
  Noise_Seed = 31415927; // arbitrary default
  Drv_Noise_Var = drv_noise_var;

  A_Coeffs = new T[ar_order+1];
  for( i=0; i<=ar_order; i++) A_Coeffs[i] = a_coeffs[i];

  Old_Output = new T[ar_order];
  for( i=0; i<ar_order; i++) 
                Old_Output[i] = 0.0;
}
template<class T>
ArSource<T>::~ArSource( void ){ };
//---------------------------
// Explicit instantiations

template ArSource<type_of_sig_vals_T>;

