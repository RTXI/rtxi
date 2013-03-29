//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  File = elipfunc.cpp
//
//  Elliptical Filter Function
//

#include <math.h>
#include <stdlib.h>
#include "misdefs.h"
#include "elipfunc.h"
#include "complex.h"
#ifdef _DEBUG
extern std::ofstream DebugFile;
#endif

//======================================================
//  constructor

EllipticalTransFunc::EllipticalTransFunc( 
                                    int order, 
                                    double passband_ripple,
                                    double stopband_ripple,
                                    double passband_edge,
                                    double stopband_edge,
                                    int upper_summation_limit )
                     :FilterTransFunc(order)
{
 int m;
 int min_order;
 double u, u4, x;
 double modular_const;
 double selec_factor;
 double double_temp;
 double discrim_factor;
 double term, sum;
 double numer, denom;
 double vv, ww, xx, yy;
 double p_sub_zero;
 double mu;
 int i, i_mirror, r;
 double aa, bb, cc;
 complex cmplx_work, work2;
 
 //------------------------------------
 // Check viability of parameter set
 
 selec_factor = passband_edge/stopband_edge;
 
 x = pow( (1.-selec_factor*selec_factor), 0.25);
 u = (1.0 - x)/(2.0*(1+x));
 u4 = u*u*u*u;
 
 //  compute:
 // modular_const = u + 2u**5 + 15u**9 + 150u**13
 
 modular_const = u*(1.+(2*u*u4*(1.+(7.5*u4*(1+(10.*u4)))))); 
 
 discrim_factor = (pow(10.0, stopband_ripple/10.0) - 1.0)/
                  (pow(10.0, passband_ripple/10.0) - 1.0);
                  
 min_order = (int)ceil( log10(16.0*discrim_factor)/
                        log10(1.0/modular_const)); 
                        
 if(order < min_order)
   {
    std::cout << "Fatal error -- minimum order of "
         << min_order << " required"
         << std::endl;
    exit(1);
   }        
   
 //------------------------------------------------------
 // compute transfer function
 
 Num_Prototype_Poles = order;
 Prototype_Pole_Locs = new complex[order+1];

 if(order%2)      //order is odd
   {Num_Prototype_Zeros = order-1;}
 else           //order is even
   {Num_Prototype_Zeros = order;}
 Prototype_Zero_Locs = 
                new complex[Num_Prototype_Zeros+1];
 
 //--------------------------------------------------
 //  step 7 Algorith 5.4
 
 r = (order - (order%2))/2;
 Num_Biquad_Sects = r;
 
 A_Biquad_Coef = new double[r+1];
 B_Biquad_Coef = new double[r+1];
 C_Biquad_Coef = new double[r+1];
 
 //-------------------------------------------------------
 //  Eq. (5.28)
 
 numer = pow(10.0, passband_ripple/20.0) + 1.0;
 
 vv = log(numer/(pow(10.0, passband_ripple/20.0)-1.))/(2.*order);
 
 //-------------------------------------------------------
 //  Eq. (5.29)
 
 sum = 0.0;
 for( m=0; m<upper_summation_limit; m++)
   {
    term = ipow(-1.0, m);
    term *= ipow(modular_const, m*(m+1));
    term *= sinh((2.*m+1)*vv);
    sum = sum + term;
   }                 
 //numer = 2.0 * sum * sqrt(sqrt(modular_const));
 numer = sum * sqrt(sqrt(modular_const));
 
 sum = 0.0;
 for( m=1; m<upper_summation_limit; m++)
   {
    term = ipow(-1.0, m);
    term *= ipow(modular_const, m*m);
    term *= cosh(2.0 * m * vv);
    sum = sum + term;
   }               
 //p_sub_zero = fabs(numer/(1.+2.*sum));
 p_sub_zero = fabs(numer/(0.5 + sum));
 
 //------------------------------------------
 //  Eq. (5.30)
 
 ww = 1.0 + selec_factor * p_sub_zero * p_sub_zero;
 ww = sqrt(ww*(1.0 + p_sub_zero * p_sub_zero / selec_factor));
 
 //---------------------------------------
 //  loop for steps 8, 9, 10, of Alg 5.4  
 
 H_Sub_Zero = 1.0;
 
 for(i=1; i<=r; i++)
   {
    if(order%2)  // if order is odd
      {  
       mu = i;
      }
    else   // order is even 
      {
       mu = i - 0.5;
      }
    //------------------------------
    //  Eq. (5.31) numerator
    
    sum = 0.0;
    for(m=0; m<upper_summation_limit; m++)
      {
       term = ipow(-1.0, m);
       term *= ipow(modular_const, m*(m+1));
       term *= sin( (2*m+1) * PI * mu /order);
       sum += term;
      }            
    numer = 2.0 * sum * sqrt(sqrt(modular_const));
    
    //---------------------------------------
    //  Eq. (5.31) denominator and finish
    
    sum = 0.0;
    for(m=1; m<upper_summation_limit; m++)
      {
       term = ipow(-1.0,m);
       term *= ipow(modular_const, m*m);
       term *= cos(2.0 * PI * m * mu/order);
       sum += term;
      }            
    xx = numer/(1.+2. * sum);
    
    //----------------------------------------
    //  Eq. (5.32)
    
    yy = 1.0 - selec_factor * xx * xx;
    yy = sqrt(yy * (1.0-(xx*xx/selec_factor)));
    
    //-----------------------------------------
    //  Eq. (5.33)
    
    aa = 1.0/(xx * xx);
    aa *= (passband_edge * passband_edge);
    A_Biquad_Coef[i] = aa;
    
    //-----------------------------------------
    //  Eq. (5.34)
    
    denom = 1.0 + ipow(p_sub_zero * xx, 2);
    bb = 2.0 * p_sub_zero * yy/denom;
    bb *= passband_edge;
    B_Biquad_Coef[i] = bb;
    
    //-----------------------------------------
    //  Eq. (5.35)
    
    denom = ipow(denom, 2);
    numer = ipow(p_sub_zero * yy, 2) + ipow(xx*ww, 2);
    cc = numer/denom;
    cc *= (passband_edge * passband_edge);
    C_Biquad_Coef[i] = cc;
    
    //--------------------------------------------
    //
    H_Sub_Zero *= (cc/aa);
    
    //-------------------------------------------
    //  compute pair of pole locations
    //  by finding roots of s**2 + bb*s + cc = 0 
    
    //------------------------------------------------------
    //  we need to compute:
    //  cmplx_work = sqrt((complex)(bb*bb - 4.*cc)); 
    //
    //  work around for missing sqrt(complex) function
    //
    //  (bb*bb - 4.0*cc) will always be real and negative
    //  so sqrt(bb*bb -4.0*cc) will always be pure imaginary
    //  equal to sqrt(-1)*sqrt(4.0*cc - bb*bb)
    //  therefore:
    
    double_temp = sqrt(4.0*cc - bb*bb);
    cmplx_work = complex(0.0, double_temp);
    
    Prototype_Pole_Locs[i] = (complex(-bb, 0.0) - cmplx_work)/2.0;
    
    #ifdef _DEBUG
    DebugFile << "in ellip response, pole[" << i << "] = "
              << Prototype_Pole_Locs[i] << std::endl;
    #endif
    
    Prototype_Pole_Locs[order+1-i] = 
                        (complex(-bb, 0.0) + cmplx_work)/2.0;
    //-----------------------------------------------------------
    // compute pair of zero locations
    // by finding roots of s**2 + a = 0
    //
    //  roots = +/- sqrt(-a)
    //
    
    if(order%2)
      {
       i_mirror = order-i;
      }
    else
      {
       i_mirror = order+1-i;
      }
    if(aa < 0.0)
      {
       double_temp = sqrt(-aa);
       Prototype_Zero_Locs[i] = complex(double_temp, 0.0);
       Prototype_Zero_Locs[i_mirror] = 
                              complex((-double_temp), 0.0);
      }
    else
      {
       double_temp = sqrt(aa);
       Prototype_Zero_Locs[i] = complex(0.0, double_temp);
       Prototype_Zero_Locs[i_mirror] =
                             complex(0.0, (-double_temp));
      }
    #ifdef _DEBUG
    DebugFile << "in ellip response, zero[" << i << "] = "
              << Prototype_Zero_Locs[i] << std::endl;
    #endif
   }
 //---------------------------
 //  Finish up Ho
 
 if(order%2)
   {
    //p_sub_zero *= stopband_edge; 
    p_sub_zero *= passband_edge; 
    H_Sub_Zero *= p_sub_zero;
    Prototype_Pole_Locs[(order+1)/2] = complex(-p_sub_zero, 0.0);
    #ifdef _DEBUG
    DebugFile << "p_sub_zero = " << p_sub_zero << std::endl;
    DebugFile << "in ellip, H_Sub_Zero = "
              << H_Sub_Zero << std::endl;
    #endif
   }
 else
   {
    H_Sub_Zero *= pow(10.0, passband_ripple/(-20.0));
   }
 return;
};


//-----------------------------------------------------
// raise double to an integer power

double ipow(double x, int m)
{
 int i;
 double result;
 result = 1.0;
 for(i=1; i<=m; i++)
   {
    result *= x;
   }
 return(result);
}
