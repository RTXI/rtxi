//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  File = pzfilt.cpp
//
//  simulation of analog pole-zero filter
//

#include <math.h>
#include "pzfilt.h"

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++
    
//======================================================
//  constructor that actually initializes filter model
//------------------------------------------------------

AnalogPoleZeroFilter::AnalogPoleZeroFilter( 
                            Polynomial numer_poly,
                            Polynomial denom_poly,
                            double h_sub_zero,
                            double delta_t)
//                    : AnalogFilter()
{
 int k, order;
 
 order = denom_poly.GetDegree();
     
 Integrator = new NumericInteg*[order];
 W_Prime = new double[order+1];
 A_Coef = new double[order+1];
 B_Coef = new double[order];
 
 Order = order;
 
 for(k=0; k<order; k++)
   {
    Integrator[k] = new NumericInteg(delta_t);
    A_Coef[k] = numer_poly.GetCoefficient(k);
    B_Coef[k] = -(denom_poly.GetCoefficient(k));
    W_Prime[k] = 0.0;
    H_Sub_Zero = h_sub_zero;
   }
 //A_Coef[order] = numer_poly.GetCoefficient(order);
 return;
};
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++      


//======================================================
//
//------------------------------------------------------

double AnalogPoleZeroFilter::Run( double input )
{
double sum, output;
int k;

sum = H_Sub_Zero * input;

for( k=0; k<Order; k++)
  {
   sum += (W_Prime[k] * B_Coef[k]);
  }
W_Prime[Order] = sum;        

//output = A_Coef[Order] * sum;
output = 0.0;

for( k=Order-1; k>=0; k--)
  {
   W_Prime[k] = ((Integrator[k])->Integrate(W_Prime[k+1]));
   output += (W_Prime[k] * A_Coef[k]);
  } 
return((float)output);
}
