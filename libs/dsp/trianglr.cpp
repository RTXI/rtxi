/*
 * Triangular window
 */

#include <stdexcept>

#include "trianglr.h"

TriangularWindow::TriangularWindow(int length, int zero_ends)
    : GenericWindow(length)
{
  GenerateWindow(length, zero_ends);
}

void TriangularWindow::GenerateWindow(int length, int zero_ends)
{
  if (length < 0) {
    throw std::invalid_argument(
        "TriangularWindow::GenerateWindow : Negative length provided");
  }
  std::vector<double> half_lag_window(GetHalfLagWindow(),
                                      GetHalfLagWindow() + GetHalfLength());
  if (zero_ends != 0) {
    if ((length % 2) != 0)  // odd length window centered at zero
    {
      for (int n = 0; n < GetHalfLength(); n++) {
        half_lag_window.at(static_cast<size_t>(n)) =
            1.0 - (2.0 * n) / ((GetLength() - 1));
      }
    } else  // even length window centered at -1/2
    {
      for (int n = 0; n < GetHalfLength(); n++) {
        half_lag_window.at(static_cast<size_t>(n)) =
            1.0 - (2.0 * n + 1.0) / ((GetLength() - 1));
      }
    }
  } else {
    if ((length % 2) != 0)  // odd length window centered at zero
    {
      for (int n = 0; n < GetHalfLength(); n++) {
        half_lag_window.at(static_cast<size_t>(n)) =
            1.0 - (2.0 * n) / ((GetLength() + 1));
      }
    } else  // even length window centered at -1/2
    {
      for (int n = 0; n < GetHalfLength(); n++) {
        half_lag_window.at(static_cast<size_t>(n)) =
            1.0 - (2.0 * n + 1.0) / ((GetLength() + 1));
      }
    }
  }
}
