//
// File = poly.h
//
#ifndef _POLY_H_
#define _POLY_H_  

#include <fstream>
#include "cmpxpoly.h"


class Polynomial
{
public: 

  //  default constructor
  Polynomial( );
  
  //  copy constructor
  Polynomial( const Polynomial &original);
  
  //  conversion constructor
  Polynomial( const CmplxPolynomial &original);
  
  // constructor for initializing a binomial
  Polynomial( const double coeff_1,
              const double coeff_0);
                   
  // assignment operator
  Polynomial& operator= (const Polynomial &right);
  
  //  multiply assign operator
  Polynomial& operator*= (const Polynomial &right);
  
  //  divide assign operator
  Polynomial& operator/= (const Polynomial &right);
  
  // dump polynomial to an output stream
  void DumpToStream( std::ofstream* output_stream);

  // get degree of polynomial
  int GetDegree(void);
  
  // return specified coefficient
  double GetCoefficient(int k);

private:
   
  int Degree;
  double* Coefficient; 
}; 
#endif 
