//
//  File = stpinvar.h
//

#ifndef _STPINVAR_H_
#define _STPINVAR_H_
#include "complex.h"

void StepInvar( complex *pole,
                int numPoles,
                complex *zero,
                int numZeros,
                double hZero,
                double bigT,
                complex *a,
                complex *b);
#endif                               
