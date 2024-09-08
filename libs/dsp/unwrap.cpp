//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  File = unwrap.cpp
//
//

#include <cmath>
#include "unwrap.hpp"

void UnwrapPhase(int ix, double& phase)
{
  static double half_circle_offset;
  static double old_phase;

  if (ix == 0) {
    half_circle_offset = 0.0;
    old_phase = phase;
    return;
  }
  phase = phase + half_circle_offset;
  if (std::fabs(old_phase - phase) > 90.0) {
    if (old_phase < phase) {
      phase = phase - 360.0;
      half_circle_offset -= 360.0;
    } else {
      phase = phase + 360.0;
      half_circle_offset += 360.0;
    }
  }
  old_phase = phase;
}
