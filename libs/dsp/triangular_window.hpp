/*
 * Triangular window
 */

#ifndef TRIANGULAR_WINDOW_HPP
#define TRIANGULAR_WINDOW_HPP

#include "generic_window.hpp"

namespace rtxi::dsp {
class TriangularWindow : public GenericWindow
{
public:
  TriangularWindow(int length, bool zero_ends);
  static std::vector<double> GenerateHalfLagWindow(int length, bool zero_ends);
};
}  // namespace rtxi::dsp
#endif
