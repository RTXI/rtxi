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

#include <cmath>
#include "gen_saw.h"

// default constructor

GeneratorSaw::GeneratorSaw()
    : m_delay(1)
    , m_width(1)
    , m_amplitude(1)
{
  setDeltaTime(1e-3);
  slopes[0] = 2 * m_amplitude / m_width;
  slopes[1] = -slopes[0];
  intersects[0] = 0;
  intersects[1] = 2 * m_amplitude;
}

GeneratorSaw::GeneratorSaw(double delay,
                           double width,
                           double amplitude,
                           double dt)
    : m_delay(delay)
    , m_width(width)
    , m_amplitude(amplitude)

{
  setDeltaTime(dt);
  slopes[0] = 2 * m_amplitude / m_width;
  slopes[1] = -slopes[0];
  intersects[0] = 0;
  intersects[1] = 2 * m_amplitude;
}

void GeneratorSaw::init(double delay, double width, double amplitude, double dt)
{
  m_delay = delay;
  m_width = width;
  m_amplitude = amplitude;
  setDeltaTime(dt);
  setIndex(0);
  slopes[0] = 2 * m_amplitude / m_width;
  slopes[1] = -slopes[0];
  intersects[0] = 0;
  intersects[1] = 2 * m_amplitude;
}

double GeneratorSaw::get()
{
  double time = getIndex() * getDeltaTime() - m_delay;
  getIndex()++;
  if (time < 0.0) {
    return 0.0;
  }
  time = fmod(time, m_width);
  return slopes[static_cast<std::size_t>(time > m_width/2)] * time
      + intersects[static_cast<std::size_t>(time > m_width/2)];
}
