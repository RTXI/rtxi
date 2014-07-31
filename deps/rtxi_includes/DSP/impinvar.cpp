//
//  file = impinvar.cpp
//

#include "impinvar.h"

void ImpulseInvar( complex *pole,
                   int num_poles,
                   complex *zero,
                   int num_zeros,
                   double h_sub_zero,
                   double big_t,
                   complex *a,
                   complex *b)
{
int k, n, j, maxCoef;
complex *delta, *big_a;
complex beta, denom, numer, work2;

delta = new complex[num_poles+1];
big_a = new complex[num_poles+1];

for(j=0; j<num_poles+1; j++)
  {
	delta[j] = complex(0.0,0.0);
	a[j] = complex(0.0,0.0);
	b[j] = complex(0.0,0.0);
	}
//---------------------------------------------------
//  compute partial fraction expansion coefficients  

for( k=1; k<=num_poles; k++) {
	numer = complex(h_sub_zero,0.0);
	for(n=1; n<=num_zeros; n++) 
		{ 
    numer = numer * (pole[n] - zero[n]);
    }
	denom = complex(1.0,0.0);
	for( n=1; n<=num_poles; n++)
    {
		if(n==k) continue;
    denom = denom * (pole[k] - pole[n]);
		}
  big_a[k] = numer/denom;
	}

//-------------------------------------
//  compute numerator coefficients

for( k=1; k<=num_poles; k++) 
  {
	delta[0] = complex(1.0, 0.0);
	for(n=1; n<=num_poles; n++)
		{
    delta[n] = complex(0.0,0.0);
    }
	maxCoef = 0;
	for( n=1; n<=num_poles; n++)
    {
		if(n==k) continue;
		  maxCoef++;
      beta = -cexp(big_t * pole[n]);
		  for(j=maxCoef; j>=1; j--) 
			  {
        delta[j] += (beta * delta[j-1]);
        }
		}
	for( j=0; j<num_poles; j++) 
		{ 
    b[j] += (big_a[k] * delta[j]);
    }
	}

//-------------------------------------
//  compute denominator coefficients 

a[0] = complex(1.0,0.0);
for( n=1; n<=num_poles; n++)
  {
  beta = -cexp(big_t * pole[n]);

	for( j=n; j>=1; j--)
		{
    a[j] += (beta * a[j-1]);
    }
	}
for( j=1; j<=num_poles; j++)
	{
  a[j] = -a[j];
  }
return;
}

