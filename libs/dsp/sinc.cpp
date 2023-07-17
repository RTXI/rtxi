//
//  File = sinc.cpp
//

#include "sinc.h"
#include <math.h>
#include <stdlib.h>

double
sinc(double x)
{
  if (x < std::numeric_limits<double>::epsilon()*2) {
    return 1.0;
  }      
  return (sin(x) / x);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
