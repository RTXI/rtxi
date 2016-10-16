/*
 * Copyright (C) 2011 Weill Medical College of Cornell University
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 675 Mass
 * Ave, Cambridge, MA 02139, USA.
 */

#include <iostream>
#include <math.h>

#include "rtmath.h"

using namespace std;

RealTimeMath::RealTimeMath()
{
  powFast = new PowFast(18);
}

RealTimeMath::~RealTimeMath()
{
  delete powFast;
}

double
RealTimeMath::fastEXP(double x)
{
  if (x > 88.5 ||
      x < -87) { // Overflow prevention due to powFast library's use of float

    // Close to 0 shortcut: If x < -746, math.h exp(x) function returns 0 due to
    // truncation
    if (x < -746)
      return 0;

    // Infinity shortcut: If x > 710, math.h exp(x) returns infinity
    if (x > 710) {
      return INFINITY;
    }

    // Standard overflow prevention
    // Due to powFast limitations from using floats, if x is too large or
    // small, problem is broken into smaller pieces
    // e.g. fastEXP(100) ==> e^180 = e^60 * e^60 * e^60
    powCount = 1;
    powAns = 1;
    powExp = x;

    while (powExp > 88.5 || powExp < -87) {
      powCount++;
      powExp = x / powCount;
    }

    for (n = 1; n <= powCount; n++) {
      powAns = powAns * powFast->e(powExp);
    }

    return powAns;
  } // end overflow prevention

  else // call powFast e^x function
    return powFast->e(x);
}

// POWER OPTIMIZATION
// x^y = e^(y*ln(x))
double
RealTimeMath::fastPOW(double x, double y)
{
  return fastEXP(y * log(x)); // Pow function using fastEXP
}
