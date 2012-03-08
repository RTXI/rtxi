//
//  File = burg.cpp
//

#include <stdlib.h>
#include <fstream>
#include "burg.h"
#include "yulewalk.h"
#include "matrix_T.h"
#include "complex.h"
#include "overload.h"

//==============================================

template < class T >
BurgMethod<T>::BurgMethod( int est_order,
                           T* x,
                           int sig_len)
{
  int i, j, k, n;
  matrix<T> kappa(1,est_order,1,est_order);
  T numer, denom;
  Ar_Order = est_order;
  Noise_Seed = 31415927; // arbitrary default

  A_Coeffs = new T[est_order+1];
  double *rho = new double[est_order+1];
  T* err_bak = new T[sig_len];
  T* err_fwd = new T[sig_len];
  T* new_bak = new T[sig_len];
  T* new_fwd = new T[sig_len];

  rho[0] = 0.0;
  for(n=0; n< sig_len; n++)
    {
    rho[0] += mag_sqrd(x[n]);
    }
  rho[0] /= sig_len;
  double awgn_adjustment;
  std::cout << "rho[0] = " << rho[0] << std::endl;
  std::cout << " enter amount to be subtracted from this value" << std::endl;
  std::cin >> awgn_adjustment;
  rho[0] -= awgn_adjustment;
  for(n=0; n< sig_len-1; n++)
    {
    err_bak[n] = x[n];
    err_fwd[n+1] = x[n+1];
    }
  for(k=1; k<=est_order; k++)
    {
    numer = 0.0;
    denom = 0.0;
    for(n=k; n<sig_len; n++)
      {
      numer += err_fwd[n] * conj(err_bak[n-1]);
      denom += mag_sqrd(err_fwd[n]) + mag_sqrd(err_bak[n-1]);
      }
    kappa[k][k] = -2.0 * numer/denom;
    rho[k] = (1.0 - mag_sqrd(kappa[k][k])) * rho[k-1];
    if(est_order==1) break;
      if(k>1)
        {
        // update coeffs for prediction error filter
        for(j=1; j<k; j++)
          kappa[j][k] = kappa[j][k-1] + kappa[k][k]
                        * conj( kappa[k-j][k-1]);
        }
      //
      // update prediction errors
      for(i=k; i<=n-2; i++)
        {
        new_bak[i] = err_bak[i-1]+conj(kappa[k][k])*err_fwd[i];
        new_fwd[i+1] = err_fwd[i+1] + kappa[k][k] * err_bak[i];
        }
      for(i=k; i<=n-2; i++)
        {
        err_bak[i] = new_bak[i];
        err_fwd[i+1] = new_fwd[i+1];
        }
    }  // end of loop over k
  Drv_Noise_Var = rho[est_order];
  for(i=1; i<=est_order; i++)
    {
    A_Coeffs[i] = kappa[i][est_order];
    std::cout << "A_Coeff[" << i << "] = " << A_Coeffs[i] << std::endl;
    }
  delete[] rho;
  delete[] err_bak;
  delete[] err_fwd;
  delete[] new_bak;
  delete[] new_fwd;
  return;
};

template < class T >
BurgMethod<T>::~BurgMethod( void ){};
//==================================================
//  Function to dump AR parameters to output
//  stream indicated by uout

template<class T>
void BurgMethod<T>::DumpParameters( ostream& uout)
  {
  uout << "Drv_Noise_Var = " << Drv_Noise_Var << std::endl;
  for(int indx=0; indx<=Ar_Order; indx++)
    {
    uout << "a[" << indx << "] = " << A_Coeffs[indx] << std::endl;
    }
  return;
  };
//==================================================
//  Function to copy AR parameters to array
//  pointed to by a_coeff

template<class T>
void BurgMethod<T>::GetParameters( int* ar_order,
                                  T* a_coeff)
  {
  *ar_order = Ar_Order;
  for(int indx=0; indx<=Ar_Order; indx++)
    {
    a_coeff[indx] = A_Coeffs[indx];
    }
  return;
  };
//---------------------------------------------------
template<class T>
double BurgMethod<T>::GetDrivingVariance( void )
{
  return(Drv_Noise_Var);
}

//template BurgMethod<complex>;
template BurgMethod<double>;

