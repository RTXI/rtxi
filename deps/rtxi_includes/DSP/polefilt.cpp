//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  File = polefilt.cpp
//
//  simulation of analog all-pole filter
//

#include <math.h>
#include "polefilt.h"

#ifdef _DEBUG
extern std::ofstream DebugFile;
#endif

//======================================================
//  constructor that actually initializes filter model
//------------------------------------------------------

AnalogAllPoleFilt::AnalogAllPoleFilt( Polynomial denom_poly,
                                      double h_sub_zero,
                                      double delta_t)
//                 : AnalogFilter()
{
 int k, order;
 
 order = denom_poly.GetDegree();
     
 Integrator = new NumericInteg*[order];
 Y_Prime = new double[order+1];
 B_Coef = new double[order];
 
 Order = order;
 H_Sub_Zero = h_sub_zero;
 
 for(k=0; k<order; k++)
   {
    Integrator[k] = new NumericInteg(delta_t);
    B_Coef[k] = -(denom_poly.GetCoefficient(k));
    #ifdef _DEBUG
    DebugFile << "in AnalogAllPoleFilt, B_Coef["
              << k << "] = " << B_Coef[k] << std::endl;
    #endif
    Y_Prime[k] = 0.0;
   }
 return;
};
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++      


//======================================================
//
//------------------------------------------------------

double AnalogAllPoleFilt::Run( double input )
{
double sum;
int k;

sum = input;
for( k=0; k<Order; k++)
  {
   sum += (Y_Prime[k] * B_Coef[k]);
  }
Y_Prime[Order] = sum;

for( k=Order-1; k>=0; k--)
  {
   Y_Prime[k] = ((Integrator[k])->Integrate(Y_Prime[k+1]));
  }    
return(H_Sub_Zero*Y_Prime[0]);
}
