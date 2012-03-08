//
// File = laguerre.h
//
#ifndef _LAGUERRE_H_
#define _LAGUERRE_H_  

#include "cmpxpoly.h"

int LaguerreMethod( CmplxPolynomial *poly,
                    complex *root_ptr,
                    double epsilon,
                    double epsilon2,
                    int max_iter);

#endif 