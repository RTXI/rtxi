//
//  File = vector_T.cpp
//

#include "complex.h"
#include "vector_T.h"
#include "matrix_T.h"

#ifdef _DEBUG
  #include <fstream>
  //#define _VEC_DEBUG 1
  extern std::ofstream DebugFile;
#endif

template<class T> class vector;
//------------------------------------------
//  constructor for row vector
template < class T >
rowvec<T>::rowvec( void )
          :vector<T>()
  {
  #ifdef _VEC_DEBUG
    DebugFile << "\nshallow ctor for rowvec at " << (void*)this 
              << std::endl;
  #endif
  //p->is_row_vec = 0;
  }

//------------------------------------------
template < class T >
rowvec<T>::rowvec(int origin, int size)
          :vector<T>(origin, size)
  {
  #ifdef _VEC_DEBUG
    DebugFile << "\nctor for rowvec at " << (void*)this 
              << "  (vrep = " << (void*)p << ")" << std::endl;
  #endif
  }
//------------------------------------------------
// transpose operator
template < class T >
rowvec<T>& colvec<T>::operator!( void )
{
  //rowvec<T> *rv = new rowvec<T>(p->orig_indx,p->length);
  rowvec<T> *rv = new rowvec<T>();
  #ifdef _VEC_DEBUG
    DebugFile << "\ncv::op!(): new rowvec at " 
              << (void*)rv << std::endl;
    DebugFile << "\ncv::op!(): hook vrep "
              << (void*)p << " to rowvec " 
              << (void*)rv << std::endl;
  #endif
  rv->p = p;
  *(rv->p)=*p;
  ((rv->p)->refcnt)++;
  rv->Is_Temp = 1;
  if(Is_Temp)
    {
    #ifdef _VEC_DEBUG
      DebugFile << "\ncv::op!(): deleting colvec at " 
                << (void*)this << std::endl;
    #endif
    delete this;
    }
  return(*rv);
}

//------------------------------------------------
// transpose operator
template < class T >
colvec<T>& rowvec<T>::operator!( void )
{
  colvec<T> *cv = new colvec<T>(p->orig_indx,p->length);
  #ifdef _VEC_DEBUG
    DebugFile << "\nnew colvec at " << (void*)cv << std::endl;
    DebugFile << "\nrv::op!(): hook vrep "
              << (void*)p << " to colvec " 
              << (void*)cv << std::endl;
  #endif
  cv->p = p;
  *(cv->p)=*p;
  cv->Is_Temp = 1;
  if(Is_Temp)
    {
    #ifdef _VEC_DEBUG
      DebugFile << "\nrv::op!(): deleting rowvec at " 
                << (void*)this << std::endl;
    #endif
    delete this;
    }
  return(*cv);
}

//---------------------------------------------------
//  row vector times column vector
template<class T>
T& rowvec<T>::operator*( colvec<T> &v2)
  {
  // get origin and length of row vector
  int v1_orig = p->orig_indx;
  int v1_len = p->length;

  // get origin and length of column vector
  int v2_orig = v2.p->orig_indx;
  int v2_len = v2.p->length;

  // alocate scalar for result
  //T *result = new T;
 // #ifdef _VEC_DEBUG
 //   DebugFile << "rv::op*(cv): new scalar alloc at "
 //             << (void*)result << std::endl;
 // #endif
  T sum;
  sum = 0;
  for(int idx=0; idx<v1_len; idx++)
    {
    //sum += ((p->f[idx+v1_orig]) * (v2.p->f[idx+v2_orig]));
    sum += ((p->f[idx]) * (v2.p->f[idx]));
    }
  //*result = sum;
  if(v2.Is_Temp)
    {
    #ifdef _VEC_DEBUG
      DebugFile << "\nrv::op*(cv): deleting colvec at " 
                << (void*)(&v2) << std::endl;
    #endif
    delete (&v2);
    }
  if(Is_Temp)
    {
    #ifdef _VEC_DEBUG
      DebugFile << "\nrv::op*(cv): deleting rowvec at " 
                << (void*)this << std::endl;
    #endif
    delete this;
    }
  //return(*result);
  return(sum);
  }
//---------------------------------------------------
//  method to multiply row vector times matrix
template<class T>
rowvec<T>& rowvec<T>::operator*( matrix<T> &m2)
{
  // check dimensions
  int vec_orig = p->orig_indx;
  int vec_len = p->length;
  int row_orig = m2._p->orig_indx;
  int nrows = m2._p->length;
  int col_orig = ((m2._p->f[row_orig])->p)->orig_indx;
  int ncols = ((m2._p->f[row_orig])->p)->length;

  if(nrows != vec_len)
    {
    #ifdef _DEBUG
      DebugFile << "error in vector method" << std::endl;
    #endif
    return(*this);
    }
  //  allocate new vector for result
  rowvec<T> *v_res = new rowvec<T>(col_orig, ncols);
  v_res->Is_Temp = 1;
  #ifdef _VEC_DEBUG
    DebugFile << "rv::op*(m): new rowvec at " 
              << (void*)v_res << std::endl;
  #endif

  // perform multiplication and populate results vector
  T sum;
  for( int j=0; j<ncols; j++)
    {
    sum = 0.0;
    for( int i=0; i<nrows; i++)
      {
      sum += ((p->f[i]) * 
             (((m2._p->f[i-(m2._p->orig_indx)])->p)->f[j]));
      }
    (v_res->p)->f[j] = sum;
    }
  if(m2.Is_Temp)
    {
    #ifdef _VEC_DEBUG
      DebugFile << "\nrv::op*(m): deleting matrix at " 
                << (void*)(&m2) << std::endl;
    #endif
    delete (&m2);
    }
  if(Is_Temp)
    {
    #ifdef _VEC_DEBUG
      DebugFile << "\nrv::op*(m): deleting rowvec at " 
                << (void*)this << std::endl;
    #endif
    delete this;
    }
  return(*v_res);
}
//------------------------------------------
//  constructor for column vector
template < class T >
colvec<T>::colvec( void )
          :vector<T>()
  {
  #ifdef _VEC_DEBUG
    DebugFile << "\nshallow ctor for colvec at " << (void*)this 
              << std::endl;
  #endif
  //p->is_row_vec = 0;
  }

//------------------------------------------
//  constructor for column vector
template < class T >
colvec<T>::colvec(int origin, int size)
          :vector<T>(origin, size)
  {
  #ifdef _VEC_DEBUG
    DebugFile << "\nctor for colvec at " << (void*)this 
              << "  (vrep = " << (void*)p << ")" << std::endl;
  #endif
  }

//---------------------------------------------------
//  method to multiply column vector times row vector
template<class T>
matrix<T>& colvec<T>::operator*( rowvec<T> &v2)
  {
  // get origin and length of column vector
  int v1_orig = p->orig_indx;
  int v1_len = p->length;

  // get origin and length of row vector
  int v2_orig = v2.p->orig_indx;
  int v2_len = v2.p->length;

  // allocate matrix for result
  matrix<T> *m_res = 
                 new matrix<T>(v1_orig,v1_len,v2_orig,v2_len);
  m_res->Is_Temp = 1;
  #ifdef _VEC_DEBUG
    DebugFile << "\ncv::op*(rv): new matrix at " 
              << (void*)m_res << std::endl;
  #endif
  for(int row=0; row<v1_len; row++)
    {
    for(int col=0; col<v2_len; col++)
      {
      ((((m_res->_p)->f[row])->p)->f[col]) = 
                                (p->f[row]) * (v2.p->f[col]);
      }
    }
  if(v2.Is_Temp)
    {
    #ifdef _VEC_DEBUG
      DebugFile << "\ncv::op*(rv): deleting rowvec at " 
                << (void*)(&v2) << std::endl;
    #endif
    delete (&v2);
    }
  if(Is_Temp)
    {
    #ifdef _VEC_DEBUG
      DebugFile << "\ncv::op*(rv): deleting colvec at " 
                << (void*)this << std::endl;
    #endif
    delete this;
    }
  return(*m_res);
  };
//------------------------------------------
template < class T >
vector<T>::vector( void )
  {
  #ifdef _VEC_DEBUG
    DebugFile << "\nctor for vector(void) at " 
              << (void*)this << std::endl;
  #endif
  //p = new vrep;
  //p->refcnt = 1;
  Is_Temp = 0;
  };

//------------------------------------------
template < class T >
void vector<T>::PurgeData(void)
  {
  #ifdef _VEC_DEBUG
    DebugFile << "\nv::PurgeData(): deleting elem array at "
              << (void*)(p->f) << std::endl;
  #endif
  if(p!=NULL)
    {
    delete[] p->f;
    #ifdef _VEC_DEBUG
      DebugFile << "\nv::PurgeData(): deleting vrep at "
                << (void*)p << std::endl;
    #endif
    delete p;
    p = NULL;
    }
  };
//------------------------------------------
template < class T >
vector<T>::vector(int origin, int size)
  {
  #ifdef _VEC_DEBUG
    DebugFile << "\nctor for vector at " << (void*)this << std::endl;
  #endif
  if( size <= 0)
      std::cout << "illegal vector dimension" << std::endl;
  p = new vrep;
  p->orig_indx = origin;
  p->length = size;
  p->max_indx = origin + size -1;
  p->f = new T[size];
  #ifdef _VEC_DEBUG
    DebugFile << "v::v(i,i): array " << size << " long alloc at "
              << (void*)(p->f) << std::endl;
  #endif
  for(int i=0; i<size; i++)
    p->f[i] = T(0.0);
  p->refcnt = 1;
  Is_Temp = 0;
  };

//-----------------------------------------
// destructor
template < class T >
rowvec<T>::~rowvec()
  {
  #ifdef _VEC_DEBUG
    DebugFile << "\ndtor for rowvec at " << (void*)this << std::endl;
  #endif
  //~vector();
  };
//-----------------------------------------
// destructor
template < class T >
colvec<T>::~colvec()
  {
  #ifdef _VEC_DEBUG
    DebugFile << "\ndtor for colvec at " << (void*)this << std::endl;
  #endif
  //~vector();
  };
//-----------------------------------------
// destructor
template < class T >
vector<T>::~vector()
  {
  #ifdef _VEC_DEBUG
    DebugFile << "\ndtor for vector at " << (void*)this << std::endl;
  #endif
  if(p!=NULL)
    {
    #ifdef _VEC_DEBUG
      DebugFile << "refcnt = " << (p->refcnt) << std::endl;
    #endif
    if(--p->refcnt == 0)
      {
      #ifdef _VEC_DEBUG
        DebugFile << "\nv::~v(): deleting elem array at "
                  << (void*)(p->f) << std::endl;
      #endif
      delete[] p->f;
      #ifdef _VEC_DEBUG
        DebugFile << "\nv::~v(): deleting vrep at "
                  << (void*)p << std::endl;
      #endif
      delete p;
      p = NULL;
      }
    }
  };
//-----------------------------------------------
// convert vector into regular array
template< class T>
T* vector<T>::array( void )
  {
  T *result;
  int len;
  len = p->length;
  result = new T[len];
  #ifdef _VEC_DEBUG
    DebugFile << "\nv::array(): alloc new array of T at " 
              << (void*)result << std::endl;
  #endif
  for(int i=0; i<len; i++)
    {
    result[i] = p->f[i];
    }
  if(Is_Temp)
    {
    #ifdef _VEC_DEBUG
      DebugFile << "\nv::array(): deleting vector at " 
                << (void*)this << std::endl;
    #endif
    delete this;
    }
  return(result);
  }
//------------------------------------------------
// "copy" constructor
template < class T >
vector<T>::vector( vector<T> &x)
{
  #ifdef _VEC_DEBUG
    DebugFile << "in copy constructor v::v(v)" << std::endl;
  #endif
  x.p->refcnt++;
  p=x.p;
}

//----------------------------------------------
// divide vector elements by a scalar
template< class T >
vector<T>& vector<T>::operator/(T denom)
{
  vector<T> *res_vec = new vector<T>(p->orig_indx, p->length);
  #ifdef _VEC_DEBUG
    DebugFile << "\nv::op/(T): new vector at " 
              << (void*)res_vec << " (vrep = "
              << (void*)(res_vec->p) << ")" << std::endl;
  #endif

  (res_vec->p)->refcnt = p->refcnt;
  (res_vec->p)->orig_indx = p->orig_indx;
  (res_vec->p)->length = p->length;
  (res_vec->p)->max_indx = p->max_indx;
  res_vec->Is_Temp = 1;
  for(int i=0; i<(p->length); i++)
    {
    //std::cout << "p->f[" << i << "] = " << (p->f[i]) << std::endl;
    res_vec->p->f[i] = ((p->f[i])/denom);
    }
  if(Is_Temp)
    {
    #ifdef _VEC_DEBUG
      DebugFile << "\nv::op/(T): deleting vector at " 
                << (void*)this << std::endl;
    #endif
    delete this;
    }
  return(*res_vec);
}
//----------------------------------------------
// assignment x = y
template < class T >
vector<T>& vector<T>::operator=(vector<T> &vec)
{
  vec.p->refcnt++;
  if(--p->refcnt == 0)
    {
    #ifdef _VEC_DEBUG
      DebugFile << "\nv::op=(v): deleting vrep at " 
                << (void*)p << std::endl;
    #endif
    delete[] p->f;
    delete p;
    p = NULL;
    }
  #ifdef _VEC_DEBUG
    DebugFile << "\nv::op=(v): hook vrep "
          << (void*)(vec.p) << " to vector " 
          << (void*)this << std::endl;
  #endif
  p = vec.p;
  if(vec.Is_Temp)
    {
    #ifdef _VEC_DEBUG
      DebugFile << "\nv::op=(v): deleting vector at " 
                << (void*)(&vec) << std::endl;
    #endif
    delete &vec;
    }
  return *this;
}

//----------------------------------------------
// assignment x = y
template < class T >
rowvec<T>& rowvec<T>::operator=(vector<T> &vec)
{
  vec.p->refcnt++;
  if(--p->refcnt == 0)
    {
    #ifdef _VEC_DEBUG
      DebugFile << "\nrv::op=(v): deleting vrep at " 
                << (void*)p << std::endl;
    #endif
    delete[] p->f;
    delete p;
    p = NULL;
    }
  #ifdef _VEC_DEBUG
    DebugFile << "\nrv::op=(v): hook vrep "
          << (void*)(vec.p) << " to vector " 
          << (void*)this << std::endl;
  #endif
  p = vec.p;
  if(vec.Is_Temp)
    {
    #ifdef _VEC_DEBUG
      DebugFile << "\nrv::op=(v): deleting vector at " 
                << (void*)(&vec) << std::endl;
    #endif
    delete &vec;
    }
  return *this;
}

//----------------------------------------------
// assignment x = y
template < class T >
colvec<T>& colvec<T>::operator=(vector<T> &vec)
{
  vec.p->refcnt++;
  if(--p->refcnt == 0)
    {
    #ifdef _VEC_DEBUG
      DebugFile << "\ncv::op=(v): deleting vrep at " 
                << (void*)p << std::endl;
    #endif
    delete[] p->f;
    delete p;
    }
  #ifdef _VEC_DEBUG
    DebugFile << "\ncv::op=(v): hook vrep "
          << (void*)(vec.p) << " to vector " 
          << (void*)this << std::endl;
  #endif
  p = vec.p;
  if(vec.Is_Temp)
    {
    #ifdef _VEC_DEBUG
      DebugFile << "\ncv::op=(v): deleting vector at " 
                << (void*)(&vec) << std::endl;
    #endif
    delete &vec;
    }
  return *this;
}

//-------------------------------------------------
//  assignment x = complex(2.0, 3.0)

template < class T >
vector<T>& vector<T>::operator=(T x)
{
  int lng;
  lng = p->length;
  if(p->refcnt > 1)
    {
    p->refcnt--;
    p = new vrep;
    p->length = lng;
    p->refcnt = 1;
    p->f = new T[lng];
    #ifdef _VEC_DEBUG
      DebugFile << "v::op=(T): array " << lng << " long alloc at "
                << (void*)(p->f) << std::endl;
    #endif
    }
  T *f = p->f;
  for(int i=0; i<lng; i++)
    *f++ = x;
  return *this;
}

template < class T >
T& vector<T>::operator[](int i)
  {
    return p->f[ (((i >=(p->orig_indx)) && (i <= p->max_indx)) ? 
                 (i-(p->orig_indx)) : 0)];
  }
//-----------------------------------------------------
//  pre-multiply matrix by a row vector
template<class T>
vector<T>& vector<T>::operator*( matrix<T> &m2)
{
  // check dimensions
  int vec_orig = p->orig_indx;
  int vec_len = p->length;
  int row_orig = m2._p->orig_indx;
  int nrows = m2._p->length;
  int col_orig = ((m2._p->f[row_orig])->p)->orig_indx;
  int ncols = ((m2._p->f[row_orig])->p)->length;

  if(nrows != vec_len)
    {
    #ifdef _VEC_DEBUG
      DebugFile << "error in vector method" << std::endl;
    #endif
    return(*this);
    }
  //  allocate new vector for result
  vector<T> *v_res = new vector<T>(col_orig, ncols);
  v_res->Is_Temp = 1;
  #ifdef _VEC_DEBUG
    DebugFile << "\nv op*(m): new rowvec at " 
              << (void*)v_res << std::endl;
  #endif

  // perform multiplication and populate results vector
  T sum;
  for( int j=0; j<ncols; j++)
    {
    sum = 0.0;
    for( int i=0; i<nrows; i++)
      {
      sum += ((p->f[i]) * 
             (((m2._p->f[i-(m2._p->orig_indx)])->p)->f[j]));
      }
    (v_res->p)->f[j] = sum;
    }
  if(m2.Is_Temp)
    {
    #ifdef _VEC_DEBUG
      DebugFile << "\nv op*(m): deleting matrix at " 
                << (void*)(&m2) << std::endl;
    #endif
    delete (&m2);
    }
  if(Is_Temp)
    {
    #ifdef _VEC_DEBUG
      DebugFile << "\nv op*(m): deleting rowvec at " 
                << (void*)this << std::endl;
    #endif
    delete this;
    }
  return(*v_res);
}
//---------------------------------
//  force desired instantiations
//template vector<complex>;
template vector<double>;
template rowvec<double>;
template colvec<double>;
template vector<complex>;
template rowvec<complex>;
template colvec<complex>;
//rowvec<double>* transpose( colvec<double>* );
