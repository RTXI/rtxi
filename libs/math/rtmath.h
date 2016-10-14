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

#include "powfast.hpp"

/*** Header Guard ***/
#ifndef REALTIMEMATH_H
#define REALTIMEMATH_H

class RealTimeMath
{

public:
  RealTimeMath();
  ~RealTimeMath();
  double fastEXP(double);
  double fastPOW(double, double);

private:
  // PowFast object
  const PowFast* powFast;
  int powCount;
  double powAns;
  double powExp;
  int n;
};

#endif
