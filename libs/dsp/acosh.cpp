/*
 * Acosh, inverse hyperbolic cosine
 */

#include <math.h>
#include <stdlib.h>
#include "acosh.h"

double
acosh(double x)
{
  double result;
  result = log(x + sqrt(x * x - 1.0));
  return (result);
}
