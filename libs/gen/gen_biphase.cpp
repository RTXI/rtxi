/*
 * Copyright (C) 2011 Georgia Institute of Technology
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "gen_biphase.h"
#include <cmath>

GeneratorBiphase::GeneratorBiphase()
    : m_width(1)
    , m_amplitude(1)
    , m_delay(1)
{
  setDeltaTime(1e-3);
}

GeneratorBiphase::GeneratorBiphase(double delay,
                                   double width,
                                   double amplitude,
                                   double dt)
    : m_width(width)
    , m_amplitude(amplitude)
    , m_delay(delay)
{
  setDeltaTime(dt);
}

void GeneratorBiphase::init(double delay,
                            double width,
                            double amplitude,
                            double dt)
{
  m_delay = delay;
  m_width = width;
  m_amplitude = amplitude;
  setDeltaTime(dt);
  setIndex(0);
}

double GeneratorBiphase::get()
{
  const double time = getIndex() * getDeltaTime() - m_delay;
  getIndex()++;
  if(time < 0.0) { return 0.0; }
  const int sign = std::fmod(time,m_width) < m_width/2 ? 1 : -1; 
  return m_amplitude * sign; 
}
