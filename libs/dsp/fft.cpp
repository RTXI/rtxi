//
//  File = fft.cpp
//

#include "fft.h"

#include "complex.h"
#include "dit_sino.h"

void ifft(complex* sample_spectrum, complex* time_signal, int num_samps)
{
  for (size_t i = 0; i < static_cast<size_t>(num_samps); i++) {
    time_signal[i] = sample_spectrum[i];
  }
  IfftDitSino(time_signal, num_samps);
}

void fft(complex* time_signal, complex* sample_spectrum, int num_samps)
{
  for (size_t i = 0; i < static_cast<size_t>(num_samps); i++) {
    sample_spectrum[i] = time_signal[i];
  }
  FftDitSino(sample_spectrum, num_samps);
}

void fft(complex* time_signal,
         complex* sample_spectrum,
         int num_samps,
         int fft_len)
{
  for (size_t i = 0; i < static_cast<size_t>(num_samps); i++) {
    sample_spectrum[i] = time_signal[i];
  }
  for (auto i = static_cast<size_t>(num_samps); i < static_cast<size_t>(fft_len); i++) {
    sample_spectrum[i] = complex(0.0, 0.0);
  }
  FftDitSino(sample_spectrum, fft_len);
}

void fft(complex* signal, int num_samps)
{
  FftDitSino(signal, num_samps);
}

