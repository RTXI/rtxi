/*
 * Hann window, not to be confused with Hanning window
 */

#include <vector>
#include <gsl/gsl_math.h>
#include <gsl/gsl_sf_trig.h>
#include "hann_window.hpp"

rtxi::dsp::HannWindow::HannWindow(int length, bool zero_ends)
    : GenericWindow(GenerateWindow(length, zero_ends))
{
}

std::vector<double> rtxi::dsp::HannWindow::GenerateWindow(int length, bool zero_ends)
{
  std::vector<double> Half_Lag_Win;
  const int even = static_cast<int>(length % 2 == 0);
  const int sign = zero_ends ? -1 : 1;
  const int half_length = even == 0 ? length / 2 : length / 2 + 1;
  Half_Lag_Win.reserve(static_cast<size_t>(half_length));
  for (int n = 0; n < half_length; n++) {
    Half_Lag_Win.push_back(0.54 + 0.46 * gsl_sf_cos(M_PI * (2 * n + even) / (length + sign)));
  }
  return Half_Lag_Win;
}
