//
//  file = complex.cpp
//

#include <math.h>
#include <iostream>

#include "complex.h"   

std::ostream& operator<<( std::ostream& s, const complex& z)
{
 s << "(" << real(z) << "," << imag(z) << ")";
 return s;
}

std::istream& operator>>( std::istream& s, complex& z)
{
  char lpar, rpar, comma;
  double _r, _i;
  for(;;)
    {
    if( (!(s >> lpar >> _r >> comma >> _i >> rpar))
        || (lpar != '(') || (rpar != ')') 
        || (comma != ',') ) 
      {
      // bad stream input
      s.clear();
      std::cout << "format for complex value is '( double, double)'\n"
           << " -- try again" << std::endl;
      continue;
      }
    break;
    }
  z = complex(_r, _i);
  return s;
}

complex operator / ( const complex value1,
                     const complex value2)
{
  double divisor = value2.re * value2.re +
                  value2.im * value2.im;
  return complex( (value2.re * value1.re +
                   value2.im * value1.im) / divisor,
                  (value1.im * value2.re -
                   value1.re * value2.im) / divisor);
}
