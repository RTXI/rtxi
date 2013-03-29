//
// File = pzfilt.h
//
#ifndef _PZFILT_H_
#define _PZFILT_H_  

#include "numinteg.h"
#include "anlgfilt.h"
#include "poly.h"

class AnalogPoleZeroFilter : public AnalogFilter
{
public: 

  //  constructors
  AnalogPoleZeroFilter( Polynomial numer_poly,
                        Polynomial denom_poly,
                        double h_sub_zero, 
                        double delta_t);
  
  double Run( double input );

private:

  NumericInteg** Integrator;
  double* W_Prime;
  int Order;
  double* A_Coef; 
  double* B_Coef;
  double H_Sub_Zero; 
}; 
#endif 