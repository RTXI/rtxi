//
// File = fft.h
//
#ifndef _FFT_H_
#define _FFT_H_

#include <vector>
#include <complex>

void fft(std::vector<std::complex<double>>& input,
         std::vector<std::complex<double>>& output);
void ifft(std::vector<std::complex<double>>& input,
          std::vector<std::complex<double>>& output);
void fft(std::vector<std::complex<double>>& signal);

#endif  // _FFT_H_
