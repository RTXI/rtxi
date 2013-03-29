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

#ifndef GEN_SAW_H_
#define GEN_SAW_H_

#include "generator.h"

class GeneratorSaw : public Generator
{

public:
  // default constructor
  GeneratorSaw();
  GeneratorSaw(double delay, double width, double amplitude, double dt);
  ~GeneratorSaw();
  // initialize waveform
  void
  init(double delay, double width, double amplitude, double dt);

protected:
  double delay; // delay in time (s) between ramps
  double width; // width in time (s) of ramp
  double amplitude;

};

#endif /* GEN_SAW_H_ */
