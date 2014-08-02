/*
 Copyright (C) 2011 Georgia Institute of Technology

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

/*
 The ZAP current stimulus is described by: 
 I(t) = Io*sin [2*pi*f(t)*t], with f(t) = fo + (fm - fo) * (t/T)
 Sample parameters: 
 fo =  0 Hz to fm = 20 Hz (2 Hz/s)
 fo = 10 Hz to fm = 40 Hz (3 Hz/s) over a total time period of T=10 s.
 In some cells we also tested the reversed protocol (reverse ZAP) where 
 frequency was varied from 20 to 0 or 40 to 10 Hz.
 */

#include "gen_zap.h"

#define TWOPI 6.28318531

// default constructor

GeneratorZap::GeneratorZap() :
  freq(1), freq2(20), duration(10), amplitude(1)
{
  index = 0;
  dt = 1e-3;
  numsamples = floor(duration / dt);
  wave.clear();
  for (int i = 0; i < numsamples; i++)
    {
      double freqtime = freq + (freq2 - freq) * (i * dt / duration);
      wave.push_back(amplitude * sin(TWOPI * freqtime * i * dt));
    }
  numsamples = wave.size();
  index = 0;
}

GeneratorZap::GeneratorZap(double freq, double freq2, double amplitude,
    double duration, double dt) :
  Generator()
{
  numsamples = floor(duration / dt);
  wave.clear();
  for (int i = 0; i < numsamples; i++)
    {
      double freqtime = freq + (freq2 - freq) * (i * dt / duration);
      wave.push_back(amplitude * sin(TWOPI * freqtime * i * dt));
    }
  numsamples = wave.size();
  index = 0;
}

GeneratorZap::~GeneratorZap()
{
}

void
GeneratorZap::init(double freq, double freq2, double amplitude,
    double duration, double dt)
{
  numsamples = floor(duration / dt);
  wave.clear();
  for (int i = 0; i < numsamples; i++)
    {
      double freqtime = freq + (freq2 - freq) * (i * dt / duration);
      wave.push_back(amplitude * sin(TWOPI * freqtime * i * dt));
    }
  numsamples = wave.size();
  index = 0;
}
