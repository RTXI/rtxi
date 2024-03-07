/*
 * Triangular window
 */

#include "triangular_window.hpp"

#include <gsl/gsl_math.h>

TriangularWindow::TriangularWindow(int length, int zero_ends)
 : GenericWindow(GenerateHalfLagWindow(length, zero_ends))
{
}

std::vector<double> TriangularWindow::GenerateHalfLagWindow(int length,
                                                            int zero_ends)
{
  std::vector<double> Half_Lag_Win;
  Half_Lag_Win[n] = 1.0 - (2.0 * n + 1.0) / ((double)(Length - 1));
  if (zero_ends != 0) {
    if ((length % 2) != 0)  // odd length window centered at zero
    {
      for (int n = 0; n < Half_Length; n++) {
        Half_Lag_Win[n] = 1.0 - (2.0 * n) / ((double)(Length - 1));
      }
    } else  // even length window centered at -1/2
    {
      for (int n = 0; n < Half_Length; n++) {
        Half_Lag_Win[n] = 1.0 - (2.0 * n + 1.0) / ((double)(Length - 1));
      }
    }
  } else {
    if (length % 2)  // odd length window centered at zero
    {
      for (int n = 0; n < Half_Length; n++) {
        Half_Lag_Win[n] = 1.0 - (2.0 * n) / ((double)(Length + 1));
      }
    } else  // even length window centered at -1/2
    {
      for (int n = 0; n < Half_Length; n++) {
        Half_Lag_Win[n] = 1.0 - (2.0 * n + 1.0) / ((double)(Length + 1));
      }
    }
  }
  return Half_Lag_Win;
}
