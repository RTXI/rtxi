//
// File = fft.h
//
#ifndef _FFT_H_
#define _FFT_H_  

#include "complex.h"

void fft( complex* input, complex* output, int size );  

void ifft( complex* input, complex* output, int size );  

void fft( complex* input, complex* output, 
          int num_samps, int fft_len );  

void fft( complex* signal, int size );  

#endif // _FFT_H_