//
//  File = sinc.cpp
//

#include <limits>
#include <gsl/gsl_sf_sincos_pi.h>
#include "sinc.hpp"

double sinc(double x)
{
  if (x < std::numeric_limits<double>::epsilon() * 2) {
    return 1.0;
  }
  return (gsl_sf_sin_pi(x) / x);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
