//
// File = polefilt.h
//
#ifndef _POLEFILT_H_
#define _POLEFILT_H_  

#include "numinteg.h"
#include "anlgfilt.h"
#include "poly.h"

class AnalogAllPoleFilt : public AnalogFilter
{
public: 

  //  constructors
  AnalogAllPoleFilt( Polynomial denom_poly, 
                     double h_sub_zero,
                     double delta_t);
  
  double Run( double input );

private:

  NumericInteg** Integrator;
  double* Y_Prime;
  int Order;
  double* B_Coef;
  double H_Sub_Zero; 
}; 
#endif 