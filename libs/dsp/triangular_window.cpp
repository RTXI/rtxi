/*
 * Triangular window
 */

#include "triangular_window.hpp"

#include <gsl/gsl_math.h>

rtxi::dsp::TriangularWindow::TriangularWindow(int length, bool zero_ends)
    : GenericWindow(GenerateHalfLagWindow(length, zero_ends))
{
}

std::vector<double> rtxi::dsp::TriangularWindow::GenerateHalfLagWindow(
    int length, bool zero_ends)
{
  std::vector<double> Half_Lag_Win;
  const int odd = length % 2;
  const int sign = zero_ends ? -1 : 1;
  const int half_length = odd != 0 ? length / 2 : length / 2 + 1;
  Half_Lag_Win.reserve(static_cast<size_t>(half_length));
  for (int n = 0; n < half_length; n++) {
    Half_Lag_Win.push_back(1.0 - (2.0 * n + odd) / (length + sign));
  }
  return Half_Lag_Win;
}
