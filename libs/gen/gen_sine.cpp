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

#include <gsl/gsl_sf_trig.h>
#include <gsl/gsl_math.h>
#include "gen_sine.h"

constexpr double TWOPI = 2 * M_PI;

GeneratorSine::GeneratorSine()
    : m_freq(1)
    , m_amplitude(1)
{
  setDeltaTime(1e-3);
}

GeneratorSine::GeneratorSine(double freq, double amplitude, double dt)
    : m_freq(freq)
    , m_amplitude(amplitude)
{
  setDeltaTime(dt);
}

double GeneratorSine::get()
{
  return m_amplitude * gsl_sf_sin(TWOPI * m_freq * getDeltaTime() * getIndex()++);
}

void GeneratorSine::init(double freq, double amplitude, double dt)
{
  m_freq = freq;
  m_amplitude = amplitude;
  setDeltaTime(dt);
  setIndex(0);
}

