//
// file = cholesky.h
//

#ifndef _CHOLESKY_H
#define _CHOLESKY_H

#include "complex.h"
#include "matrix_t.h"

int CholeskyDecomp(int ord, matrix<complex>* ax, complex* bx);
// int CholeskyDecomp( int ord,
//                    complex_matrix *ax,
//                    complex *bx,
//                    double epsilon );

#endif //_CHOLESKY_H
