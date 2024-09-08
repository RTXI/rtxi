/*
 * Dolph-Chebyshev window
 */

#include "dolph.hpp"

#include <gsl/gsl_math.h>

DolphChebyWindow::DolphChebyWindow(size_t length, double atten)
{
  double Alpha_Parm = cosh(acosh(pow(10.0, atten / 20.0)) / (length - 1));
  GenerateWindow(length, Alpha_Parm);
}

void DolphChebyWindow::GenerateWindow(size_t length, double Alpha_Parm)
{
  double denom = NAN;
  double numer = NAN;
  double x = NAN;
  double sum_re = NAN;
  double sum_im = NAN;
  size_t num_freq_samps = 0;
  size_t beg_freq_idx = 0;
  size_t end_freq_idx = 0;
  std::vector<double> freq_resp(length * Interp_Rate);
  size_t interp_rate = 10;

  beg_freq_idx = ((1 - length) * interp_rate) / 2;
  end_freq_idx = ((length - 1) * interp_rate) / 2;
  num_freq_samps = interp_rate * (length - 1) + 1;

  denom = cosh(static_cast<double>(length) * acosh(Alpha_Parm));

  for (size_t n = beg_freq_idx; n <= end_freq_idx; n++) {
    x = Alpha_Parm
        * cos((M_PI * static_cast<double>(n))
              / static_cast<double>(interp_rate * length));
    if (x < 1.0) {
      numer = std::cos(static_cast<double>(length) * std::acos(x));
    } else {
      numer = std::cosh(static_cast<double>(length) * std::acosh(x));
    }
    if (n < 0) {
      freq_resp.at(n + num_freq_samps) = numer / denom;
    } else {
      freq_resp.at(n) = numer / denom;
    }
  }
  //  now do inverse DFT

  std::vector<double> result;
  result.reserve(length);
  for (size_t n = 0; n <= (length - 1) / 2; n++) {
    sum_re = 0.0;
    sum_im = 0.0;

    for (size_t k = 0; k < num_freq_samps; k++) {
      sum_re +=
          (freq_resp.at(k)
           * cos((M_PI * 2 * k * n) / static_cast<double>(num_freq_samps)));
      sum_im +=
          (freq_resp.at(k)
           * sin((M_PI * 2 * k * n) / static_cast<double>(num_freq_samps)));
    }
    result.at(n) = sum_re / static_cast<double>(num_freq_samps);
  }
  SetDataWindow(result);
  NormalizeWindow();
}
