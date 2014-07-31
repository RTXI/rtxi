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

#include "generator.h"

Generator::Generator() :
  index(0), numsamples(1), dt(1e-3)
{
  wave.clear();
  wave.push_back(0);
}

Generator::~Generator()
{
}

void
Generator::clear()
{
  wave.clear();
  wave.push_back(0);
  index = 0;
  numsamples = wave.size();
}

void
Generator::init()
{
  wave.clear();
  wave.push_back(0);
  index = 0;
  numsamples = wave.size();
}

double
Generator::get()
{
  double value = wave[index];
  index++;
  if (index >= numsamples)
    index = 0;
  return value;

}

double
Generator::getOne()
{
  double value = wave[index];
  index++;
  if (index >= numsamples)
    value = 0;
  return value;

}
int
Generator::numSamples() const
{
  return numsamples;
}

int
Generator::getIndex() const
{
  return index;
}

void
Generator::setIndex(int value)
{
  index = value;
}
