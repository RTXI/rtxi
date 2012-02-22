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

#include "gen_mono.h"

// default constructor

GeneratorMono::GeneratorMono() :
  delay(1), width(1), amplitude(1)
{
  dt = 1e-3;
  numsamples = floor(delay / dt) + 2 * floor(width / dt) + 1;
  wave.clear();
  for (int i = 0; i < floor(delay / dt); i++)
    {
      wave.push_back(0); // initial delay
    }
  for (int i = 0; i < floor(width / dt); i++)
    {
      wave.push_back(amplitude); // positive part
    }
  numsamples = wave.size();
  index = 0;
}

GeneratorMono::GeneratorMono(double delay, double width, double amplitude,
    double dt) :
  Generator()
{
  numsamples = floor(delay / dt) + 2 * floor(width / dt) + 1;
  wave.clear();
  for (int i = 0; i < floor(delay / dt); i++)
    {
      wave.push_back(0); // initial delay
    }
  for (int i = 0; i < floor(width / dt); i++)
    {
      wave.push_back(amplitude); // positive part
    }
  numsamples = wave.size();
  index = 0;
}

GeneratorMono::~GeneratorMono()
{
}

void
GeneratorMono::init(double delay, double width, double amplitude, double dt)
{
  numsamples = floor(delay / dt) + 2 * floor(width / dt) + 1;
  wave.clear();
  for (int i = 0; i < floor(delay / dt); i++)
    {
      wave.push_back(0); // initial delay
    }
  for (int i = 0; i < floor(width / dt); i++)
    {
      wave.push_back(amplitude); // positive part
    }
  numsamples = wave.size();
  index = 0;
}
