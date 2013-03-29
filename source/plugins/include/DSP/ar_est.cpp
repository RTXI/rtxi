//
//  File = ar_est.cpp
//

#include <stdlib.h>
#include <fstream>
#include "ar_est.h"
#include "yulewalk.h"
#include "sig_type.h"

//=====================================================
//  ArEstimate - subclass of ArProcess for case where
//               AR coefficients must be estimated from
//               observed data

template < class T >
ArEstimate<T>::ArEstimate( int est_ar_order,
                           T* signal,
                           int sig_len)
              : ArProcess<T>()
{
  int i, err_stat;
  Ar_Order = est_ar_order;
  Noise_Seed = 31415927; // arbitrary default

  //-------------------------------------------
  // initialize model state

  Old_Output = new T[est_ar_order];
  for( i=0; i<est_ar_order; i++) 
                Old_Output[i] = 0.0;

  //-------------------------------------------
  // solve Yule-Walker equations

  A_Coeffs = new T[est_ar_order+1];
 
  YuleWalker<T>* yw_ptr = 
                    new YuleWalker<T>( signal,
                                       sig_len,
                                       est_ar_order,
                                       A_Coeffs,
                                       &Drv_Noise_Var,
                                       &err_stat);

  return;
}

template < class T >
ArEstimate<T>::~ArEstimate( void ){};

//==================================================
//  Function to dump estimated AR parameters to output
//  stream indicated by uout

template<class T>
void ArEstimate<T>::DumpParameters( ostream& uout)
  {
  uout << "estim. Drv_Noise_Var = " << Drv_Noise_Var << std::endl;
  for(int indx=0; indx<=Ar_Order; indx++)
    {
    uout << "estimated a[" << indx << "] = " << A_Coeffs[indx] << std::endl;
    }
  return;
  }

template ArEstimate<type_of_sig_vals_T>;

