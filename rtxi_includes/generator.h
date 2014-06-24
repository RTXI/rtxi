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

#ifndef GENERATOR_H_
#define GENERATOR_H_

#include <math.h>
#include <vector>

class Generator
{
public:
  // default constructor
  Generator();
  ~Generator();
  // clear the waveform
  void
  clear();
  // initialize waveform
  void
  init();
  // get readout for continuous signal, repeating cycle
  double
  get();
  // get readout for single cycle
  double
  getOne();
  // get number of samples
  int
  numSamples() const;
  // get current index for readout
  int
  getIndex() const;
  // set index
  void
  setIndex(int value);

protected:
  std::vector<double> wave;
  int index;
  int numsamples;
  double dt;

};

#endif /* GENERATOR_H_ */
