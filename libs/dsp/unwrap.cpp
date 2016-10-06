//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  File = unwrap.cpp
//
//

#include "unwrap.h"
#include <fstream>
#include <math.h>
#include <stdlib.h>

//========================================
//
void
UnwrapPhase(int ix, double* phase)
{
  static double half_circle_offset;
  static double old_phase;

  if (ix == 0) {
    half_circle_offset = 0.0;
    old_phase = *phase;
  } else {
    *phase = *phase + half_circle_offset;
    if (fabs(old_phase - *phase) > double(90.0)) {
      if (old_phase < *phase) {
        *phase = *phase - 360.0;
        half_circle_offset -= 360.0;
      } else {
        *phase = *phase + 360.0;
        half_circle_offset += 360.0;
      }
    }
    old_phase = *phase;
  }
  return;
}
