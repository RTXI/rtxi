//
//  File = stpinvar.cpp
//

#include "stpinvar.h"

void StepInvar( complex *pole,
                int num_poles,
                complex *zero,
                int num_zeros,
                double h_sub_zero,
                double big_t,
                complex *a,
                complex *b)
{
int k, n, j, max_coef;
complex *delta, *big_a;
complex beta, denom, numer, work2;

delta = new complex[num_poles+1];
big_a = new complex[num_poles+1];

for(j=0; j<=num_poles; j++) {
	delta[j] = complex(0.0,0.0);
	a[j] = complex(0.0,0.0);
	b[j] = complex(0.0,0.0);
	}
pole[0] = complex(0.0,0.0);

//---------------------------------------------------
//  compute partial fraction expansion coefficients

for( k=0; k<=num_poles; k++)
  {
	numer = complex(h_sub_zero,0.0);
	for(n=1; n<=num_zeros; n++) 
		{
    numer *= (pole[n] - zero[n]);
    }
	denom = complex(1.0,0.0);
	for( n=0; n<=num_poles; n++)
    {
		if(n==k) continue;
      denom *= (pole[k] - pole[n]);
		}
  big_a[k] = numer/denom;
	}

//-------------------------------------
//  compute numerator coefficients 

for( k=1; k<=num_poles; k++)
  {
	delta[0] = complex(1.0, 0.0);
	for(n=1; n<=num_poles; n++)
		{delta[n] = complex(0.0,0.0);}
	max_coef = 0;
	for( n=0; n<=num_poles; n++) {
		if(n==k) continue;
		max_coef++;
    beta = -cexp(big_t * pole[n]);

		for(j=max_coef; j>=1; j--) 
			{
      delta[j] += (beta * delta[j-1]);
      }
		}
	for( j=0; j<num_poles; j++) 
		{ 
    b[j] += (big_a[k] * delta[j]);
    }

// multiply by 1-z**(-1) 
	beta = complex(-1.0,0.0);
	for(j=num_poles+1; j>=1; j--)
    {
    b[j] += (beta * b[j-1]);
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
