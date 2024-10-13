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

/*
 * The ZAP current stimulus is described by:
 *   I(t) = Io*sin [2*pi*f(t)*t], with f(t) = fo + (fm - fo) * (t/T)
 *
 * Sample parameters:
 *   fo =  0 Hz to fm = 20 Hz (2 Hz/s)
 *   fo = 10 Hz to fm = 40 Hz (3 Hz/s) over a total time period of T=10 s.
 *
 * In some cells we also tested the reversed protocol (reverse ZAP) where
 * frequency was varied from 20 to 0 or 40 to 10 Hz.
 */

#include <gsl/gsl_sf_trig.h>
#include <gsl/gsl_math.h>
#include "gen_zap.h"

constexpr double TWOPI = 2 * M_PI;

// default constructor

GeneratorZap::GeneratorZap()
    : m_freq(1)
    , m_freq2(20)
    , m_duration(10)
    , m_amplitude(1)
{
  setDeltaTime(1e-3);
}

GeneratorZap::GeneratorZap(
    double freq, double freq2, double amplitude, double duration, double dt)
    : m_freq(freq)
    , m_freq2(freq2)
    , m_duration(duration)
    , m_amplitude(amplitude)
{
  setDeltaTime(dt);
}

void GeneratorZap::init(
    double freq, double freq2, double amplitude, double duration, double dt)
{
  setDeltaTime(dt);
  m_freq = freq;
  m_freq2 = freq2;
  m_duration = duration;
  m_amplitude = amplitude;
  setIndex(0);
}

double GeneratorZap::get()
{
  const double dt = getDeltaTime();
  if(getIndex() > m_duration / dt) { setIndex(0); }
  const double freqtime = m_freq + (m_freq2 - m_freq) * ( getIndex() * dt / m_duration);
  return m_amplitude * gsl_sf_sin(TWOPI * freqtime * dt * getIndex()++);
}

