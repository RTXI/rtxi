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

#include "gen_saw.h"

// default constructor

GeneratorSaw::GeneratorSaw() :
  delay(1), width(1), amplitude(1)
{
  dt = 1e-3;
  numsamples = floor(width / 2 / dt) + 1;
  double inc = amplitude / numsamples;
  wave.clear();
  for (int i = 0; i < floor(delay / dt); i++)
    {
      wave.push_back(0); // initial delay
    }
  while (wave.back() < amplitude)
    {
      wave.push_back(wave.back() + inc); // up
    }
  while (wave.back() > 0)
    {
      wave.push_back(wave.back() - inc); // down
    }
  numsamples = wave.size();
  index = 0;
}

GeneratorSaw::GeneratorSaw(double delay, double width, double amplitude, double dt) :
  Generator()
{
  numsamples = floor(width / 2 / dt) + 1;
  double inc = amplitude / numsamples;
  wave.clear();
  for (int i = 0; i < floor(delay / dt); i++)
    {
      wave.push_back(0); // initial delay
    }
  while (wave.back() < amplitude)
    {
      wave.push_back(wave.back() + inc); // up
    }
  while (wave.back() > 0)
    {
      wave.push_back(wave.back() - inc); // down
    }
  numsamples = wave.size();
  index = 0;
}

GeneratorSaw::~GeneratorSaw()
{
}

void
GeneratorSaw::init(double delay, double width, double amplitude, double dt)
{
  numsamples = floor(width / 2 / dt) + 1;
  double inc = amplitude / numsamples;
  wave.clear();
  for (int i = 0; i < floor(delay / dt); i++)
    {
      wave.push_back(0); // initial delay
    }
  while (wave.back() < amplitude)
    {
      wave.push_back(wave.back() + inc); // up
    }
  while (wave.back() > 0)
    {
      wave.push_back(wave.back() - inc); // down
    }
  numsamples = wave.size();
  index = 0;
}
