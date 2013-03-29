//
//  File = bilinear.cpp
//                     

 #include <stdlib.h> 
 #include <iostream>
 #include <fstream> 
 #include "misdefs.h"
 #include "typedefs.h" 
 #include "complex.h"
 #include "bilinear.h"
 #ifdef _DEBUG
 extern std::ofstream DebugFile;
 #endif
 
//====================================================
//
//----------------------------------------------------
IirFilterDesign* BilinearTransf( 
                         FilterTransFunc* analog_filter,
                         double sampling_interval)

{
 int max_poles;
 int num_poles, num_zeros;
 int j, m, n;
 int max_coeff, num_numer_coeff;
 double h_const, h_sub_zero, denom_mu_zero;
 complex *pole, *zero;
 complex *mu;
 complex alpha;
 complex beta;
 complex gamma;
 complex delta;
 complex eta;
 complex work;
 complex c_two;
 double *a, *b;
 IirFilterDesign* iir_filter;
 
 pole = analog_filter->GetPoles(&num_poles);
 zero = analog_filter->GetZeros(&num_zeros);
 h_sub_zero = analog_filter->GetHSubZero();
 #ifdef _DEBUG
 DebugFile << "num analog poles = " << num_poles << std::endl;
 DebugFile << "num analog zeros = " << num_zeros << std::endl;
 #endif

 if(num_poles > num_zeros)
   { max_poles = num_poles; }
 else
   { max_poles = num_zeros; }
   
 //--------------------------------------------
 // allocate and initialize working storage
 
 mu = new complex[max_poles+1];
 a = new double[max_poles+1];
 b = new double[max_poles+1];
 
 for(j=0; j<=max_poles; j++)
   {
    mu[j] = complex(0.0, 0.0);
    a[j] = 0.0;
    b[j] = 0.0;
   }           
   
 //-------------------------------------------
 // compute constant gain factor
 
 h_const = 1.0;
 work = complex(1.0, 0.0);
 c_two = complex(2.0, 0.0);
 
 for(n=1; n<=num_poles; n++)
   {
    work = work * (c_two - (sampling_interval * pole[n]));
    #ifdef _DEBUG
    DebugFile << "work = " << work << std::endl;
    #endif
    h_const = h_const * sampling_interval;
   } 
 #ifdef _DEBUG
 DebugFile << "T**2 = " << h_const << std::endl;                         
 #endif
 h_const = h_sub_zero * h_const / real(work);
 
 #ifdef _DEBUG
 DebugFile << "in BilinearTransf, h_const = "
           << h_const << std::endl;
 DebugFile << "work = " << work << std::endl;
 #endif

 //--------------------------------------------------
 // compute denominator coefficients
 
 mu[0] = complex(1.0, 0.0);
 
 for(n=1; n<=num_poles; n++)
   {
    #ifdef _DEBUG
    DebugFile << "in BilinearTransf, pole [" << n
              << "] = " << pole[n] << std::endl;
    #endif
    
    gamma = complex( (2.0/sampling_interval), 0.0) - pole[n];
    delta = complex( (-2.0/sampling_interval), 0.0) - pole[n];
    
    #ifdef _DEBUG
    DebugFile << "gamma = " << gamma << std::endl;
    DebugFile << "delta = " << delta << std::endl;
    #endif
    
    for(j=n; j>=1; j--)
      {
       mu[j] = gamma * mu[j] + (delta * mu[j-1]);
      }
    mu[0] = gamma * mu[0];
   } 
 #ifdef _DEBUG
 DebugFile << "for denom, mu[0] = " << mu[0] << std::endl;  
 #endif
 denom_mu_zero = real(mu[0]);                                
 for( j=1; j<=num_poles; j++)
   {
    a[j] = -1.0 * real(mu[j])/denom_mu_zero;
    #ifdef _DEBUG
    DebugFile << "a[" << j << "] = " << a[j] << std::endl;
    DebugFile << "imag(mu[" << j << "]) = "
              << imag(mu[j]) << std::endl;
    #endif
   }
 //-----------------------------------------------------
 //  compute numerator coeffcients
 
 mu[0] = complex(1.0, 0.0);
 for(n=1; n<=max_poles; n++)
   {
    mu[n] = complex(0.0, 0.0);
   }                                 
   
max_coeff = 0;
 
 //- - - - - - - - - - - - - - - - - - - - -
 //  compute (1+z**(-1)) ** (N-M)
 
 for(m=1; m<=(num_poles-num_zeros); m++)
   {
    max_coeff++;
    for(j=max_coeff; j>=1; j--)
      {
       mu[j] = mu[j] + mu[j-1];
      }
   }
 for(m=1; m<=num_zeros; m++)
   {
    max_coeff++;
    #ifdef _DEBUG
    DebugFile << "zero[" << m << "] = " << zero[m] << std::endl;
    #endif
    alpha = complex( (2.0/sampling_interval), 0.0) - zero[m];
    beta = complex( (-2.0/sampling_interval), 0.0) - zero[m]; 
    
    for(j=max_coeff; j>=1; j--)
      {
       mu[j] = alpha * mu[j] + (beta * mu[j-1]);
      }
    mu[0] = alpha * mu[0];
   }
 num_numer_coeff = max_coeff+1;
 for(j=0; j<num_numer_coeff; j++)
   {
    b[j] = h_sub_zero * real(mu[j])/denom_mu_zero;
    #ifdef _DEBUG
    DebugFile << "b[" << j << "] = " << b[j] << std::endl;
    DebugFile << "imag(mu[" << j << "]) = " 
              << imag(mu[j]) << std::endl;
    #endif
   }                             
 
 delete []mu; 
 iir_filter = new IirFilterDesign( num_numer_coeff, 
                                   num_poles,
                                   b, a);
 iir_filter->SetSamplingInterval(sampling_interval);                 
 return(iir_filter);
} 
