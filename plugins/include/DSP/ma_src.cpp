//
//  File = ma_src.cpp
//

#include <stdlib.h>
#include <fstream>
#include "ma_src.h"

#include "sig_type.h" //selects signal type for template 
                      //instantiation

#ifdef _DEBUG
  extern std::ofstream DebugFile;
#endif

//======================================================
//  MaSource - subclass of MaProcess for the case where 
//             the AR coefficients are known or assumed 
//             and are provided as input parameters

template < class T >
MaSource<T>::MaSource( int ma_order,
                       T *b_coeffs,
                       double drv_noise_var )
            :MaProcess<T>()
{
  int i;
  Ma_Order = ma_order;
  Noise_Seed = 31415927; // arbitrary default
  Drv_Noise_Var = drv_noise_var;

  B_Coeffs = new T[ma_order+1];
  for( i=0; i<=ma_order; i++) B_Coeffs[i] = b_coeffs[i];

  Old_Input = new T[ma_order+1];
  for( i=0; i<=ma_order; i++) 
                Old_Input[i] = 0.0;
}

template < class T >
MaSource<T>::~MaSource( void ){}

//-----------------------------------------------
//  Explicit instantiations
//template MaSource<double>;
template MaSource < type_of_sig_vals_T >;