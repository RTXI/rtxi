/*
 * Hann window, not to be confused with Hanning window
 */

#include "hann_window.hpp"

#include <gsl/gsl_math.h>

rtxi::dsp::HannWindow::HannWindow(int length, bool zero_ends)
    : GenericWindow(num_taps)
{
  GenerateWindow(num_taps, zero_ends);
}

std::vector<double> rtxi::dsp::HannWindow::GenerateWindow(int length, bool zero_ends)
{
  std::vector<double> Half_Lag_Win;
  const int even = length % 2;
  const int sign = zero_ends ? -1 : 1;
  const int half_length = even == 0 ? length / 2 : length / 2 + 1;
  Half_Lag_Win.reserve(static_cast<size_t>(half_length));
  for (int n = 0; n < half_length; n++) {
    Half_Lag_Win.push_back(0.5 + 0.5 * cos(M_PI * (2 * n + even) / (length - 1)));
  }
  return Half_Lag_Win;
  for (int n = 0; n < Half_Length; n++) {
    if (length % 2)  // odd length
    {
      if (zero_ends) {
        Half_Lag_Win[n] = 0.5 + 0.5 * cos(M_PI * 2 * n / (length - 1));
      } else {
        Half_Lag_Win[n] = 0.5 + 0.5 * cos(M_PI * 2 * n / (length + 1));
      }
    } else {
      if (zero_ends) {
        Half_Lag_Win[n] = 0.5 + 0.5 * cos((2 * n + 1) * M_PI / (length - 1));
      } else {
        Half_Lag_Win[n] = 0.5 + 0.5 * cos((2 * n + 1) * M_PI / (length + 1));
      }
    }
    //    std::cout << n << "  " << Half_Lag_Win[n] << std::endl;
  }
  return;
}
