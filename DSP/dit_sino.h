//
// File = dit_sino.h
//
#ifndef _DIT_SINO_H_
#define _DIT_SINO_H_  

#include "complex.h"

void FftDitSino( complex* array, int fft_size); 
void IfftDitSino( complex* array, int fft_size); 
 
#endif // _DIT_SINO_H_