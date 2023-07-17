//
//  File = sincsqrd.cpp
//

#include "sinc.h"
#include "sincsqrd.h"
#include <math.h>
#include <stdlib.h>

double
sinc_sqrd(double x)
{
  double result = sinc(x);
  return result*result;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
