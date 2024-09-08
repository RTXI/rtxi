//
// File = laguerre.h
//
#ifndef _LAGUERRE_H_
#define _LAGUERRE_H_

#include <complex>
#include <vector>

int LaguerreMethod(const std::vector<std::complex<double>>& coeff,
                   std::complex<double>& root,
                   double epsilon,
                   double epsilon2,
                   int max_iter);

#endif
