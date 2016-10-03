//
//  File = toeplitz.h
//

#ifndef _TOEPLITZ_H_
#define _TOEPLITZ_H_

#include "complex.h"
#include "matrix.h"

template <class T>
class ToeplitzMatrix : public Matrix<T>
{
public:
  ToeplitzMatrix( );

  T* GetCol(int col_indx);

protected:
  T *Herm_Toep_Col_1;

};

#endif
