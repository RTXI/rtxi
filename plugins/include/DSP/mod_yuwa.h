//
//  File = mod_yuwa.h
//

#ifndef _MOD_YUWA_H_
#define _MOD_YUWA_H_

#include "toeplitz.h"
#include "complex.h"
#include "typedefs.h"
#include "matrix.h"

template <class T>
class ModYuleWalker
{
public:
  ModYuleWalker( T *signal,
                int seq_len,
                int ar_order,
                int ma_order);
  ~ModYuleWalker(void);
  T* GetCoeffs(void);
  double GetDrivingVariance(void);

private:
  Matrix<T> *Correl_Matrix;
  T *Toeplitz_Corr_Matrix;
  T *A_Vec;
  double Ar_Drv_Var; 
  int Ar_Order;
  int Ma_Order;
};

#endif
