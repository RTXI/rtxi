/*
 * Hann window, not to be confused with Hanning window
 */

#include <stdexcept>
#include <algorithm>
#include <gsl/gsl_math.h>
#include <gsl/gsl_sf_trig.h>
#include "hann.h"

HannWindow::HannWindow(int num_taps, int zero_ends)
    : GenericWindow(num_taps)
{
  GenerateWindow(num_taps, zero_ends);
}

void HannWindow::GenerateWindow(int length, int zero_ends)
{
  if (length < 0) {
    throw std::invalid_argument(
        "HannWindow::GenerateWindow : Negative length provided");
  }
  std::vector<double> half_lag_win(static_cast<size_t>(GetHalfLength()));
  for (int n = 0; n < GetHalfLength(); n++) {
    if ((length % 2) != 0)  // odd length
    {
      if (zero_ends != 0) {
        half_lag_win.at(static_cast<size_t>(n)) =
            0.5 + 0.5 * gsl_sf_cos(M_PI * 2 * n / (length - 1));
      } else {
        half_lag_win.at(static_cast<size_t>(n)) =
            0.5 + 0.5 * gsl_sf_cos(M_PI * 2 * n / (length + 1));
      }
    } else {
      if (zero_ends != 0) {
        half_lag_win.at(static_cast<size_t>(n)) =
            0.5 + 0.5 * gsl_sf_cos((2 * n + 1) * M_PI / (length - 1));
      } else {
        half_lag_win.at(static_cast<size_t>(n)) =
            0.5 + 0.5 * gsl_sf_cos((2 * n + 1) * M_PI / (length + 1));
      }
    }
  }
  std::iter_swap(GetHalfLagWindow(), half_lag_win.data());
}
