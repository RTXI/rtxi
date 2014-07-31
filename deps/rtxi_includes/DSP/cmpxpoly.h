//
// File = cmpxpoly.h
//
#ifndef _CMPXPOLY_H_
#define _CMPXPOLY_H_  

#include <fstream>
#include "complex.h"


class CmplxPolynomial
{
public: 

  //  default constructor
  CmplxPolynomial( );
  
  //  copy constructor
  CmplxPolynomial( const CmplxPolynomial &original);
  
  // constructor for initializing a binomial
  CmplxPolynomial( const complex coeff_1,
                   const complex coeff_0);
                   
  //  initializing constructor
  CmplxPolynomial( const complex *coeff,
                   const int degree);
                   
  //  initializing constructor
  CmplxPolynomial( const double *coeff,
                   const int degree);
                   
  // assignment operator
  CmplxPolynomial& operator= (const CmplxPolynomial &right);
  
  //  multiply assign operator
  CmplxPolynomial& operator*= (const CmplxPolynomial &right);
  
  //  divide assign operator
  CmplxPolynomial& operator/= (const CmplxPolynomial &divisor);
  
  // return array of polynomial root values
  complex* GetRoots( void );

  // reflect root across the unit circle
  void ReflectRoot( int root_idx );

  // dump polynomial to an output stream
  void DumpToStream( std::ostream* output_stream);
  
  // get degree of polynomial
  int GetDegree(void);
  
  // return specified coefficient
  complex GetCoeff(int k);

  // return pointer to copy of coefficients
  void CopyCoeffs(complex*);

  friend class Polynomial;

private:

  // find roots of the polynomial
  void FindRoots( void );

  // build sum of powers coefficients from roots
  void BuildFromRoots( void );
   
  int Degree;
  complex* Coeff; 
  complex* RemCoeff;
  complex* Root;
}; 
#endif 
