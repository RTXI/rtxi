//
//  File = matrix.h
//

#ifndef _MATRIX_H_
#define _MATRIX_H_

template <class T>
class Matrix
{
public:
  Matrix( ){};

  virtual T* GetCol(int col_indx)=0;
  //virtual double ProcessSample( double input_val )=0;
  //virtual long ProcessSample( long input_val )=0;

protected:
  int Num_Rows;
  int Num_Cols;
};

#endif
