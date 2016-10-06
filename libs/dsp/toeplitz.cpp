//
//  File = toeplitz.cpp
//

#include <fstream>
#include <stdlib.h>
#include <iostream>

#include "toeplitz.h"

template <class T>
ToeplitzMatrix<T>::ToeplitzMatrix(void)
{
  return;
}

template <class T>
T*
ToeplitzMatrix<T>::GetCol(int col_indx)
{
  // right now this only returns column 1
  int indx;
  T *col_vect, *src_ptr, *dest_ptr;
  col_vect = new T[this->Num_Rows];
  dest_ptr = col_vect;
  src_ptr = Herm_Toep_Col_1;
  for (indx = 0; indx < this->Num_Rows; indx++) {
    *dest_ptr = *src_ptr;
    dest_ptr++;
    src_ptr++;
  }
  return (col_vect);
}
template class ToeplitzMatrix<double>;
template class ToeplitzMatrix<complex>;
