//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  File = chebfunc.cpp
//
//  Chebyshev Filter Function
//

#include <math.h>
#include "misdefs.h"
#include "chebfunc.h"
#include "complex.h"

//======================================================
//  constructor

ChebyshevTransFunc::ChebyshevTransFunc( int order,
                                        double ripple,
                                        int ripple_bw_norm )
                   :FilterTransFunc(order)
{
 double x;
 int k;
 double epsilon, gamma;
 double big_r, big_a;
 double sigma_mult, omega_mult;
 complex work;
 
 Prototype_Pole_Locs = new complex[order+1];
 Num_Prototype_Poles = order;
 Prototype_Zero_Locs = new complex[1];
 Num_Prototype_Zeros = 0;
 
 epsilon = sqrt(pow(10.0, (double)(ripple/10.0)) -1.0);
 gamma = pow( (1+sqrt(1.0 + epsilon*epsilon))/epsilon,
              1.0/(double)order); 
 if(ripple_bw_norm)
   {
    big_r = 1.0;
   }
 else
   {
    big_a = log((1.0+sqrt(1.0-epsilon*epsilon))/epsilon)/order;
    big_r = (exp(big_a)+exp(-big_a))/2.0;
    std::cout << "big_r = " << big_r << std::endl; 
   }
              
 sigma_mult = ( (1.0/gamma) - gamma) / (2.0 * big_r);
 
 omega_mult = ( (1.0/gamma) + gamma) / (2.0 * big_r);
 
 for(k=1; k<=order; k++)
   {
    x = PI * ((2*k)-1) / (2*order);
    
    Prototype_Pole_Locs[k] = complex( sigma_mult * sin(x),
                                             omega_mult * cos(x) );
   }
 //------------------------------------------------
 //  compute gain factor Ho
 
 work = complex(1.0, 0.0);
 for(k=1; k<=order; k++)
   {
    work *= (-Prototype_Pole_Locs[k]);
   }
 
 H_Sub_Zero = real(work);
 
 if(order%2 == 0)  // if order is even
   {
    H_Sub_Zero /= sqrt(1.0 + epsilon*epsilon);
   }
  
 return;
};
