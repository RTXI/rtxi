//
//  File = uni_rand.cpp
//
#include "uni_rand.h"

#define MULTIPLIER 16807
#define MODULUS 2147483647
#define MOD_RECIP (1.0/MODULUS)

#define SCHRAGE_QUOTIENT 127773
//      equals integer part of (MODULUS/MULTIPLIER)

#define SCHRAGE_REMAINDER 2836
//      equals MODULUS - (SCHRAGE_QUOTIENT * MULTIPLIER)

#define MASK 123459876

float UniformRandom(long *seed)
{
 long temp;
 float result;

 temp = (*seed)/SCHRAGE_QUOTIENT;
 *seed = MULTIPLIER * (*seed - temp * SCHRAGE_QUOTIENT)
         - SCHRAGE_REMAINDER * temp;
 if(*seed<0) *seed += MODULUS;

 // get floating point value from seed
 result = float( MOD_RECIP * (*seed));
 
 return(result);
 }
 
 
double DoubleUniformRandom(long *seed)
{
 long temp;
 double result;
 temp = (*seed)/SCHRAGE_QUOTIENT;
 *seed = MULTIPLIER * (*seed - temp * SCHRAGE_QUOTIENT)
         - SCHRAGE_REMAINDER*temp;
 if(*seed<0) *seed += MODULUS;
 result = MOD_RECIP * (*seed);
 
 return(result);
 }
