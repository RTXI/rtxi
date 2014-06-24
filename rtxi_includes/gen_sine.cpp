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

#include "gen_sine.h"

#define TWOPI 6.28318531

// default constructor

GeneratorSine::GeneratorSine() :
  freq(1), amplitude(1)
{
  index = 0;
  dt = 1e-3;
  numsamples = floor(1 / freq / dt);
  wave.clear();
  for (int i = 0; i < numsamples; i++)
    {
      wave.push_back(amplitude * sin(TWOPI * freq * i * dt));
    }
  numsamples = wave.size();
  index = 0;
}

GeneratorSine::GeneratorSine(double freq, double amplitude, double dt) :
  Generator()
{
  numsamples = floor(1 / freq / dt);
  wave.clear();
  for (int i = 0; i < numsamples; i++)
    {
      wave.push_back(amplitude * sin(TWOPI * freq * i * dt));
    }
  numsamples = wave.size();
  index = 0;
}

GeneratorSine::~GeneratorSine()
{
}

void
GeneratorSine::init(double freq, double amplitude, double dt)
{
  numsamples = floor(1 / freq / dt);
  wave.clear();
  for (int i = 0; i < numsamples; i++)
    {
      wave.push_back(amplitude * sin(TWOPI * freq * i * dt));
    }
  numsamples = wave.size();
  index = 0;
}
