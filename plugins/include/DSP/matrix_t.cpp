//
//  File = matrix_T.cpp
//

#include <error.h>
#include "matrix_T.h"

#ifdef _DEBUG
  #include <fstream.h>
  extern ofstream DebugFile;
  //#define _MTX_DEBUG 1
#endif

//---------------------------------------------
//  constructor
template< class T >
matrix<T>::matrix( int row_orig, 
                   int nrows, 
                   int col_orig,
                   int ncols)
  {
  if( (nrows <= 0) || (ncols <=0))
      cout << "illegal matrix dimension" << endl;
  _p = new mrep;
  #ifdef _MTX_DEBUG
  DebugFile << "\nctor for matrix at " << this 
            << "   (mrep = " << (void*)_p << ")" << endl;
  #endif
  _p->length = nrows;
  _p->orig_indx = row_orig;
  _p->max_indx = row_orig + nrows - 1;
  _p->f = new rowvec<T>*[nrows];
  #ifdef _MTX_DEBUG
    DebugFile << "v::v(i,i): ptr array " << nrows 
              << " long alloc at "
              << (void*)(_p->f) << endl;
  #endif
  for(int i=0; i<nrows; i++)
    _p->f[i] = new rowvec<T>( col_orig, ncols);
  _p->refcnt = 1;
  Is_Temp = 0;
  }

//----------------------------------------------
template<class T>
matrix<T>::~matrix(void)
{
  //rowvec<T> *row_ptr;

  #ifdef _MTX_DEBUG
  DebugFile << "\ndtor for matrix at " << (void*)this << endl;
  #endif
  if( --_p->refcnt == 0)
    {
    int nrows = _p->length;
    for(int i=0; i<nrows; i++)
            delete _p->f[i];
    delete _p->f;
    #ifdef _MTX_DEBUG
    DebugFile << "\nm::~m(): deleting mrep at " 
              << (void*)_p << endl;
    #endif
    delete _p;
    }
}
//----------------------------------------------
// row extraction
template < class T >
rowvec<T>& matrix<T>::operator[](int i)
{
  return *(_p->f[ (((i>=(_p->orig_indx)) && (i<= _p->max_indx)) ? 
                 (i-(_p->orig_indx)) : 0)]);
}

//--------------------------------------------------
//  post-multiply matrix by a column vector
template < class T >
colvec<T>& matrix<T>::operator*( colvec<T> &v2)
{
  // check dimensions
  int row_orig = _p->orig_indx;
  int nrows = _p->length;
  int col_orig = ((_p->f[_p->orig_indx])->p)->orig_indx;
  int ncols = ((_p->f[_p->orig_indx])->p)->length;
  int vec_orig = v2.p->orig_indx;
  int vec_len = v2.p->length;
  
  if(ncols != vec_len)
    {
    cout << "error in matrix method" << endl;
    return( v2 );
    }

  // allocate new vector for result
  colvec<T> *v_res = new colvec<T>(row_orig, nrows);
  #ifdef _MTX_DEBUG
  DebugFile << "\nm::op*(cv): new colvec at "
            << (void*)v_res << endl;
  #endif
  v_res->Is_Temp = 1;

  // perform multiplication and populate results vector
  T sum;
  for(int i=0; i<nrows; i++)
    {
    sum = 0.0;
    for(int j=0; j<vec_len; j++)
      {
      //sum += ((v2.p->f[j]) * (((_p->f[i-(_p->orig_indx)])->p)->f[j]));
      sum += ((v2.p->f[j]) * (((_p->f[i])->p)->f[j]));
      }
    (v_res->p)->f[i] = sum; 
    }
  if(v2.Is_Temp)
    {
    #ifdef _MTX_DEBUG
    DebugFile << "\nm::op*(cv): deleting colvec at " 
              << (void*)(&v2) << endl;
    #endif
    delete (&v2);
    }
  if(Is_Temp)
    {
    #ifdef _MTX_DEBUG
    DebugFile << "\nm::op*(cv): deleting matrix at " 
              << (void*)this << endl;
    #endif
    delete this;
    }
  return (*v_res);
}
//--------------------------------------------------
//  do element-by-element subtraction
template< class T>
matrix<T>& matrix<T>::operator-=(matrix<T> &m2)
  {
  int nrows = _p->length;
  int ncols = ((_p->f[_p->orig_indx])->p)->length;
  for(int i=0; i<nrows; i++)
    {
    for(int j=0; j<ncols; j++)
      {
      ((_p->f[i])->p)->f[j] -= (((m2._p)->f[i])->p)->f[j];
      }
    }
  if(m2.Is_Temp)
    {
    #ifdef _MTX_DEBUG
    DebugFile << "\nm::op-=(m): deleting matrix at " 
              << (void*)(&m2) << endl;
    #endif
    delete (&m2);
    }
  return(*this);
  }
template matrix<double>;
template matrix<complex>;
