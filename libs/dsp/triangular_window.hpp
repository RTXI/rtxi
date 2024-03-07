/*
 * Triangular window
 */

#ifndef TRIANGULAR_WINDOW_HPP
#define TRIANGULAR_WINDOW_HPP

#include "generic_window.hpp"

class TriangularWindow : public GenericWindow
{
public:
  // constructors

  TriangularWindow(int length, int zero_ends);
  std::vector<double> GenerateHalfLagWindow(int length, int zero_ends);

  // private:

  // int Num_Taps;
  // double *Half_Lag_Window;
};

#endif
