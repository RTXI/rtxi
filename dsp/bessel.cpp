/*
 * Bessel function of the first kind and zero order
 */

#include "bessel.h"
#include <stdlib.h>

double
bessel_I_zero(double x)
{
  double sum, numer, denom, term, half_x;
  sum = 1.0;
  half_x = x / 2.0;
  denom = 1.0;
  numer = 1.0;
  for (int m = 1; m <= 32; m++) {
    numer *= half_x;
    denom *= m;
    term = numer / denom;
    sum += term * term;
  }
  return (sum);
}
