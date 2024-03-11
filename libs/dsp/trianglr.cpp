/*
 * Triangular window
 */

#include "trianglr.h"

TriangularWindow::TriangularWindow(int num_taps, int zero_ends)
  : GenericWindow(num_taps)
{
  GenerateWindow(num_taps, zero_ends);
}

void
TriangularWindow::GenerateWindow(int length, int zero_ends)
{
  
  if (zero_ends != 0) {
    if ((length % 2) != 0) // odd length window centered at zero
    {
      for (int n = 0; n < GetHalfLength(); n++) {
        GetHalfLagWindow()[n] = 1.0 - (2.0 * n) / ((GetLength() - 1));
      }
    } else // even length window centered at -1/2
    {
      for (int n = 0; n < GetHalfLength(); n++) {
        GetHalfLagWindow()[n] = 1.0 - (2.0 * n + 1.0) / ((GetLength() - 1));
      }
    }
  } else {
    if ((length % 2) != 0) // odd length window centered at zero
    {
      for (int n = 0; n < GetHalfLength(); n++) {
        GetHalfLagWindow()[n] = 1.0 - (2.0 * n) / ((GetLength() + 1));
      }
    } else // even length window centered at -1/2
    {
      for (int n = 0; n < Half_Length; n++) {
        Half_Lag_Win[n] = 1.0 - (2.0 * n + 1.0) / ((double)(Length + 1));
      }
    }
  }
  return;
}
