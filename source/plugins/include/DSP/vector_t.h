//
//  File = vector_T.h
//

#ifndef _VECTOR_T_H_
#define _VECTOR_T_H_

#include "complex.h"
#include "matrix_T.h"

//=======================================
template<class T> class matrix;
template<class T> class rowvec;
template<class T> class colvec;
template< class T>
class vector
{
protected:
  struct vrep
    {
    T *f;
    int refcnt;
    int orig_indx;
    int length;
    int max_indx;
    } *p;
  int Is_Temp;

  vector(void);
public:
  //vector( int origin, int size, int is_row_vec);
  vector( int origin, int size);
  vector(vector<T> &x);
  void PurgeData( void);
  ~vector(void);
  vector<T>& operator=(vector<T>& vec);
  vector<T>& operator=(T x);
  vector<T>& operator/(T x);
  T *array(void);
  T& operator[](int);
  vector<T>& operator*( matrix<T> &m2);
  //vector<T>& operator*( matrix &m2);
  //friend vector<T>& matrix<T>::operator*( vector<T> &v2);
  friend class matrix<T>;
  friend class rowvec<T>;
  friend class colvec<T>;

};

template< class T>
class rowvec : public vector<T>
{
public:
  // constructors
  rowvec( void );
  ~rowvec( void);
  rowvec( int origin, int size );

  // row vector times matrix
  rowvec<T>& operator*(matrix<T> &m2);

  rowvec<T>& operator=(vector<T> &vec);

  // transpose operator
  colvec<T>& operator!();

  // row vector times column vector
  T& operator*(colvec<T> &v2);
  //T operator*(colvec<T> &v2);
  friend class colvec<T>;
  //friend rowvec<T>* transpose( colvec<T> *x);
  //friend colvec<T>& transpose( rowvec<T> &x);
};

//=======================================
template< class T>
class colvec : public vector<T>
{
public:
  // constructor
  colvec( void );
  colvec( int origin, int size );
  ~colvec( void );

  // column vector time row vector
  matrix<T>& operator*(rowvec<T> &v2);

  colvec<T>& operator=(vector<T> &vec);

  // transpose operator
  rowvec<T>& operator!();

  friend class rowvec<T>;
  //friend rowvec<T>* transpose( colvec<T> *x);
  //friend colvec<T>& transpose( rowvec<T> &x);
};


template<class T>
rowvec<T>* transpose( colvec<T>* );

#endif
