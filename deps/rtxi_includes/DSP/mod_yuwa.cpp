//
//  File = mod_yuwa.cpp
//

#include <stdlib.h>
#include <fstream>
#include "autometh.h"
#include "gen_lev.h"
#include "yulewalk.h"
#include "mod_yuwa.h"
#include "matrix_T.h"

#ifdef _DEBUG
  extern std::ofstream DebugFile;
#endif

template <class T>
ModYuleWalker<T>::ModYuleWalker( T *signal,
                                int seq_len,
                                int ar_order,
                                int ma_order)
{
  int err_stat;
  double epsilon=0.0001;

  matrix<T> a_work( 1, ar_order, 1, ar_order);
  matrix<T> b_work( 1, ar_order, 1, ar_order);

  Ar_Order = ar_order;
  Ma_Order = ma_order;
  A_Vec = new T[ar_order+1];
  Correl_Matrix = 
         new AutocorrMethCorrMtx<T>( signal, 
                                     seq_len, 
                                     ar_order + ma_order + 1);
  Toeplitz_Corr_Matrix = Correl_Matrix->GetCol(1);

  err_stat = GeneralizedLevinson( Toeplitz_Corr_Matrix,
                                  ar_order,
                                  ma_order,
                                  epsilon,
                                  A_Vec);
  #ifdef _DEBUG
    DebugFile << "returned to ModYuleWalker" << std::endl;
    DebugFile << "err_stat = " << err_stat << std::endl;
    for(int indx=0; indx<=ar_order; indx++)
      {
      DebugFile << "A_Vec[ " << indx << " ] = " 
                << A_Vec[indx] << std::endl;
      }
  #endif
  return;
}

template <class T>
ModYuleWalker<T>::~ModYuleWalker( void)
{
  delete Correl_Matrix;
  delete[] Toeplitz_Corr_Matrix;
  delete[] A_Vec;
  return;
}

template <class T>
T* ModYuleWalker<T>::GetCoeffs(void)
{
  T *a_vec, *src_ptr, *dest_ptr;
  src_ptr = A_Vec;
  a_vec = new T[Ar_Order+1];
  dest_ptr = a_vec;
  for(int i=0; i<=Ar_Order; i++)
  {
    *dest_ptr = *src_ptr;
    dest_ptr++;
    src_ptr++;
  }
  return(a_vec);
}

template <class T>
double ModYuleWalker<T>::GetDrivingVariance(void)
{
  return(Ar_Drv_Var);
};

template ModYuleWalker<double>;
template ModYuleWalker<complex>;