/*
 * Hann window, not to be confused with Hanning window
 */

#ifndef HANN_WINDOW_HPP
#define HANN_WINDOW_HPP

#include <vector>
#include "generic_window.hpp"

namespace rtxi::dsp{
class HannWindow : public rtxi::dsp::GenericWindow
{
public:
  HannWindow(int length, bool zero_ends);
  std::vector<double> GenerateWindow(int length, bool zero_ends);
};
}  // namespace rtxi::dsp
#endif
