//
//  File = armaproc.cpp
//

#include <stdlib.h>
#include <fstream>
#include "arma_src.h"
#include "gausrand.h"
#include "yulewalk.h"
#include "mod_yuwa.h"

//=======================================================
//  ArmaSource - subclass of ArmaProcess for case where 
//               AR coefficients are known or assumed and 
//               are provided as input parameters

template <class T>
ArmaSource<T>::ArmaSource( int ar_order,
                           T *a_coeffs,
                           int ma_order,
                           T *b_coeffs,
                           double drv_noise_var )
              :ArmaProcess<T>()
{
  int i;
  Ar_Order = ar_order;
  Ma_Order = ma_order;
  Noise_Seed = 31415927; // arbitrary default
  Drv_Noise_Var = drv_noise_var;

  A_Coeffs = new T[ar_order+1];
  for( i=0; i<=ar_order; i++) A_Coeffs[i] = a_coeffs[i];

  B_Coeffs = new T[ma_order+1];
  for( i=0; i<=ma_order; i++) B_Coeffs[i] = b_coeffs[i];

  Old_Input = new T[ma_order+1];
  for( i=0; i<=ma_order; i++) 
                Old_Input[i] = 0.0;
  Old_Output = new T[ar_order];
  for( i=0; i<ar_order; i++)
                Old_Output[i] = 0.0;
}
//-------------------------------------
//  Explicit instantiations of template
template ArmaSource<double>;
template ArmaSource<complex>;

