//
// File = fft.h
//
#ifndef _FFT_H_
#define _FFT_H_

#include "complex.h"

/*!
  * Fast Fourier Transform
  *
  * \param time_signal Input signal in time domain
  * \param sample_spectrum Output spectrum in frequency domain
  * \param num_samps Number of samples in the buffers
  *
  * \sa ifft()
  */
void fft(complex* time_signal, complex* sample_spectrum, int num_samps);

/*!
  * Inverse Fast Fourier Transform
  *
  * \param sample_spectrum Input spectrum in frequency domain
  * \param time_signal Output signal in time domain
  * \param num_samps Number of samples in the buffers
  *
  * \sa fft()
  */
void ifft(complex* sample_spectrum, complex* time_signal, int num_samps);

/*!
  * Fast Fourier Transform
  *
  * \param time_signal Input signal in time domain
  * \param sample_spectrum Output spectrum in frequency domain
  * \param num_samps Number of samples in the input signal 
  * \param fft_len Length of the fft spectrum in frequency bins
  *
  * \sa ifft()
  */
void fft(complex* time_signal, complex* sample_spectrum, int num_samps, int fft_len);

/*!
  * Inverse Fast Fourier Transform
  *
  * \param time_signal Input signal in time domain
  * \param num_samps Number of samples in the input signal 
  *
  * \sa ifft()
  */
void fft(complex* signal, int num_samps);

#endif // _FFT_H_
