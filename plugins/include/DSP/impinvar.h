//
//  File = impinvar.h
//

#ifndef _IMPINVAR_H_
#define _IMPINVAR_H_
#include "complex.h"

void impulseInvar( complex *pole,
                   int numPoles,
                   complex *zero,
                   int numZeros,
                   double hZero,
                   double bigT,
                   complex *a,
                   complex *b);
#endif                               
