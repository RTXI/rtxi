#ifndef _MATRIX_T_H_
#define _MATRIX_T_H_

#include "vector_T.h"
template<class T> class rowvec;
template<class T> class colvec;
template<class T> class vector;
template<class T>
class matrix
{
protected:
  struct mrep
    {
    rowvec<T> **f;
    int refcnt;
    int orig_indx;
    int length;
    int max_indx;
    } *_p;
  int Is_Temp;
public:
  matrix<T>( int row_orig, int nrows, 
             int col_orig, int ncols);
  ~matrix<T>( void );
  rowvec<T>& operator[](int i);
  colvec<T>& operator*( colvec<T> &v2);
  matrix<T>& operator-=( matrix<T> &m2);
  friend class vector<T>;
  friend class rowvec<T>;
  friend class colvec<T>;
  ///friend vector<T>& vector<T>::operator*( matrix<T> &m2);
  //void operator=(const double_complex& right);
};
//vector<double>& operator*(matrix<double> &m1, vector<double> &v2);
//matrix<double>& operator*(vector<double> &m1, vector<double> &v2);

#endif