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

#ifndef GEN_SINE_H_
#define GEN_SINE_H_

#include "generator.h"

class GeneratorSine : public Generator
{

public:
  // default constructor
  GeneratorSine();
  GeneratorSine(double freq, double amplitude, double dt);
  ~GeneratorSine();
  // initialize waveform
  void
  init(double freq, double amplitude, double dt);

protected:
  double freq; // Hz
  double amplitude;

};

#endif /* GEN_SINE_H_ */
