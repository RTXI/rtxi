

#include "fft.hpp"

#include <gsl/gsl_complex.h>
#include <gsl/gsl_fft.h>
#include <gsl/gsl_fft_complex.h>

#include "debug.hpp"

void ifft(std::vector<std::complex<double>>& input,
          std::vector<std::complex<double>>& output)
{
  if (input.size() != output.size()) {
    ERROR_MSG("dsp::ifft : input and output arrays do not match!");
    return;
  }
  std::copy(input.begin(), input.end(), output.begin());
  int err =
      gsl_fft_complex_radix2_transform(reinterpret_cast<double*>(output.data()),
                                       1,
                                       output.size(),
                                       gsl_fft_backward);
  if (err != 0) {
    ERROR_MSG("dsp::fft : Unable to perform inverse transform");
  }
}

void fft(std::vector<std::complex<double>>& input,
         std::vector<std::complex<double>>& output)
{
  if (input.size() > output.size()) {
    ERROR_MSG(
        "dsp::ifft : input buffer size must be smaller or equal to the output "
        "buffer size");
    return;
  }
  std::copy(input.begin(), input.end(), output.begin());
  int err =
      gsl_fft_complex_radix2_transform(reinterpret_cast<double*>(output.data()),
                                       1,
                                       output.size(),
                                       gsl_fft_forward);
  if (err != 0) {
    ERROR_MSG("dsp::fft : Unable to perform forward transform");
  }
}

void fft(std::vector<std::complex<double>>& signal)
{
  int err = gsl_fft_complex_radix2_forward(
      reinterpret_cast<double*>(signal.data()), 1, signal.size());
  if (err != 0) {
    ERROR_MSG("dsp::fft : Unable to perform forward transform");
  }
}
