//
// file = fsk_spec.cpp
//

 #include <stdlib.h>  
 #include <iostream>
 #include <fstream>
 #include <math.h> 
 #include "misdefs.h"
 #include "sinc.h"
 #include "fsk_spec.h"

 CpfskSpectrum::CpfskSpectrum( int big_m, 
                               double f_d, 
                               double big_t)
 {
 double psi, f, phi, alpha_mn;
 double a_m, a_n, b_mn, x_m, x_n;
 double sum, sum2;
 double freq_incr;
 int n, m, freq_idx;

 Big_M = big_m;
 Freq_Dev = f_d;
 Big_T = big_t;

 sum = 0.0;
 for(n=1; n<=big_m/2; n++)
  {
  sum += (cos(2*PI*f_d*big_t*(2*n-1)));
  }
  psi = 2.0 * sum/big_m;

  freq_incr = 0.125;
  for(freq_idx=0; freq_idx<128; freq_idx++)
    {
    f=freq_idx * freq_incr;
    sum = 0.0;
    sum2 = 0.0;
    for(n=1; n<=big_m; n++)
      {
      x_n = PI * big_t * (f-(2*n-1-big_m)*f_d);
      a_n = sinc(x_n);
      sum += a_n * a_n;

      for(m=1; m<=big_m; m++)
        {
        x_m = PI * big_t * (f-(2*m-1-big_m)*f_d);
        a_m = sinc(x_m);

        alpha_mn = TWO_PI * f_d * big_t * (m+n-1-big_m);

        b_mn = (cos(TWO_PI * f * big_t - alpha_mn)
               - psi * cos(alpha_mn))/
               (1.0 + psi*psi - 2 * psi 
               * cos(TWO_PI * f * big_t));
        sum2 += b_mn * a_n * a_m;
        } // end of loop over m
      }  // end of loop over n
    phi = big_t*(sum/2.0 + (sum2 /big_m))/big_m;
    if(freq_idx == 0) std::cout << "at f=0, phi = " << phi << std::endl;
    } // end of loop over freq_idx
 }
 double CpfskSpectrum::GetPsdValue( double freq )
 {
 double psi, phi, alpha_mn;
 double a_m, a_n, b_mn, x_m, x_n;
 double sum, sum2;
 int n, m;
 
 sum = 0.0;
 for(n=1; n<=Big_M/2; n++)
  {
  sum += (cos(2*PI*Freq_Dev*Big_T*(2*n-1)));
  }
  psi = 2.0 * sum/Big_M;

  sum = 0.0;
  sum2 = 0.0;
  for(n=1; n<=Big_M; n++)
    {
    x_n = PI * Big_T * (freq-(2*n-1-Big_M)*Freq_Dev);
    a_n = sinc(x_n);
    sum += a_n * a_n;

    for(m=1; m<=Big_M; m++)
      {
      x_m = PI * Big_T * (freq-(2*m-1-Big_M)*Freq_Dev);
      a_m = sinc(x_m);

      alpha_mn = TWO_PI * Freq_Dev * Big_T * (m+n-1-Big_M);

      b_mn = (cos(TWO_PI * freq * Big_T - alpha_mn)
             - psi * cos(alpha_mn))/
             (1.0 + psi*psi - 2 * psi 
             * cos(TWO_PI * freq * Big_T));
      sum2 += b_mn * a_n * a_m;
      } // end of loop over m
    }  // end of loop over n
  phi = Big_T*(sum/2.0 + (sum2 /Big_M))/Big_M;
  return(phi);
 }
 