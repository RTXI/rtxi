 //
 //  File = levin.cpp
 //

#include "complex.h"
#include <fstream>
#include "levin.h"
#include "overload.h"

#ifdef _DEBUG
  extern std::ofstream DebugFile;
#endif

 int LevinsonRecursion( double *toeplitz,
                        int ar_order,
                        double *avec,
                        double *ar_drv_var_ret)
 {
  double sum, work, temp;
  double ar_drv_var;
  int j, k, kdiv2, kminj;

  ar_drv_var = real(toeplitz[0]);
  avec[0] = 1.0;
  for(k=1; k<=ar_order; k++) avec[k]=0.0;

  for(k=1; k<=ar_order; k++)
    {
    sum = toeplitz[k];
    for(j=1; j<k; j++)
      {
      sum += avec[j] * toeplitz[k-j];
      }
    work = -sum/ar_drv_var;
    ar_drv_var *= (1.0 - mag_sqrd(work));

    avec[k] = work;
    kdiv2 = k/2;
    for(j=1; j<=kdiv2; j++)
      {
      kminj = k-j;
      temp = avec[j];
      avec[j] = temp + work*conj(avec[kminj]);
      if(j==kminj) continue;
        avec[kminj] += work * conj(temp);
      }
    #ifdef _DEBUG
       DebugFile << "k = " << k << "  ar_drv_var = "
                 << ar_drv_var << std::endl;
    #endif
    } // end of loop over k
  *ar_drv_var_ret = ar_drv_var;
  return(0);
 };
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 int LevinsonRecursion( complex *toeplitz,
                        int ar_order,
                        complex *avec,
                        double *ar_drv_var_ret)
 {
  complex sum, work, temp;
  double ar_drv_var;
  int j, k, kdiv2, kminj;

  ar_drv_var = real(toeplitz[0]);
  avec[0] = 1.0;
  for(k=1; k<=ar_order; k++) avec[k]=0.0;

  for(k=1; k<=ar_order; k++)
    {
    sum = toeplitz[k];
    for(j=1; j<k; j++)
      {
      sum += avec[j] * toeplitz[k-j];
      }
    work = -sum/ar_drv_var;
    ar_drv_var *= (1.0 - mag_sqrd(work));
/*
    if(ar_drv_var <= 0.0)
      {
      *ar_drv_var_ret = ar_drv_var;
      return(1);
      } */
    avec[k] = work;
    kdiv2 = k/2;
    for(j=1; j<=kdiv2; j++)
      {
      kminj = k-j;
      temp = avec[j];
      avec[j] = temp + work*conj(avec[kminj]);
      if(j==kminj) continue;
        avec[kminj] += work * conj(temp);
      }
    #ifdef _DEBUG
      DebugFile << "k = " << k << "  ar_drv_var = "
                << ar_drv_var << std::endl;
    #endif
    } // end of loop over k
  *ar_drv_var_ret = ar_drv_var;
  return(0);
 };
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 #ifdef _NOT_DEFINED
 template <class T>
 int LevinsonRecursion( T *toeplitz,
                        int ar_order,
                        T *avec,
                        double *ar_drv_var_ret)
 {
  T sum, work, temp;
  double ar_drv_var;
  int j, k, kdiv2, kminj;

  ar_drv_var = real(toeplitz[0]);
  avec[0] = 1.0;
  for(k=1; k<=ar_order; k++) avec[k]=0.0;

  for(k=1; k<=ar_order; k++)
    {
    sum = toeplitz[k];
    for(j=1; j<k; j++)
      {
      sum += avec[j] * toeplitz[k-j];
      }
    work = -sum/ar_drv_var;
    ar_drv_var *= (1.0 - mag_sqrd(work));
/*
    if(ar_drv_var <= 0.0)
      {
      *ar_drv_var_ret = ar_drv_var;
      return(1);
      } */
    avec[k] = work;
    kdiv2 = k/2;
    for(j=1; j<=kdiv2; j++)
      {
      kminj = k-j;
      temp = avec[j];
      avec[j] = temp + work*conj(avec[kminj]);
      if(j==kminj) continue;
        avec[kminj] += work * conj(temp);
      }
    #ifdef _DEBUG
      DebugFile << "k = " << k << "  ar_drv_var = "
                << ar_drv_var << std::endl;
    #endif
    } // end of loop over k
  *ar_drv_var_ret = ar_drv_var;
  return(0);
 };
 //
 // Force instantiations
 //template LevinsonRecursion <complex>;
 #endif
