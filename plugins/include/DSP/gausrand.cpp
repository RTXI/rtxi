//
//  File = gausrand.cpp
//

#include <math.h>
#include "uni_rand.h"
#include "gausrand.h"
 
double GaussRandom(long *seed)
{
 double u1, u2, g1; 
 double UA, UB, S;
 double radical;

 new_start:
 UA = DoubleUniformRandom(seed);
 UB = DoubleUniformRandom(seed);
 u1 = 1.0 - 2.0 * UA;
 u2 = 1.0 - 2.0 * UB;
 S = u1*u1 + u2*u2;
 if(S >= 1.0 ) goto new_start;
 radical = sqrt(-2.0*log(S)/S);
 g1 = u1 * radical;
 return(g1);
}
