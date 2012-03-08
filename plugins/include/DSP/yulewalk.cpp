//
//  File = yulewalk.cpp
//

#include <stdlib.h>
#include <fstream>
#include "autometh.h"
#include "levin.h"
#include "yulewalk.h"

#ifdef _DEBUG
  extern std::ofstream DebugFile;
#endif

template <class T>
YuleWalker<T>::YuleWalker( T *signal,
                           int seq_len,
                           int ar_ord,
                           T *a_vec,
                           double *drv_noise_var,
                           int *err_stat)
{
  int idb;
  Matrix<T> *corr_mtx;
  corr_mtx = new AutocorrMethCorrMtx<T>( signal, 
                                         seq_len, 
                                         ar_ord);

  T *correl_vec, sum;
  correl_vec = corr_mtx->GetCol(1);
  #ifdef _DEBUG
    for(idb=0; idb<=ar_ord; idb++)
    {
     DebugFile << "R[" << idb << "] = " << correl_vec[idb] << std::endl;
    }
  #endif
  *err_stat = LevinsonRecursion( correl_vec,
                                 ar_ord,
                                 a_vec,
                                 drv_noise_var);
  std::cout << "err_stat = " << (*err_stat) << std::endl;

  sum = 0.0;
  for(idb=1; idb<=ar_ord; idb++)
    {
    sum += a_vec[idb] * correl_vec[idb];
    }
  sum = correl_vec[0] - sum;
  return;
}

template <class T>
YuleWalker<T>::YuleWalker( T *toep_corr_mtx,
                           int ar_ord,
                           T *a_vec,
                           double *drv_noise_var,
                           int *err_stat)
{
  *err_stat = LevinsonRecursion( toep_corr_mtx,
                                 ar_ord,
                                 a_vec,
                                 drv_noise_var);
  return;
}
template YuleWalker<double>;
template YuleWalker<complex>;
