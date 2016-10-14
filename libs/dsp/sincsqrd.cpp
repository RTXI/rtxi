//
//  File = sincsqrd.cpp
//

#include "sincsqrd.h"
#include <math.h>
#include <stdlib.h>

double
sinc_sqrd(double x)
{
  double result;
  if (x == 0.0) {
    result = 1.0;
  } else {
    result = sin(x) / x;
    result = result * result;
  }
  return (result);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
