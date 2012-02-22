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

#ifndef GEN_WHITENOISE_H_
#define GEN_WHITENOISE_H_

#include "generator.h"
#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>

class GeneratorWNoise : public Generator
{

public:

  // default constructor
  GeneratorWNoise();
  GeneratorWNoise(double variance);
  ~GeneratorWNoise();
  // initialize waveform
  void
  init(double variance);
  // get readout
  double
  get();
  // set the variance of the generator
  void
  setVariance(double var);

protected:
  double variance;
  double s;
  double u1;
  double u2;
  double v1;
  double v2;
  double x;
  double y;
  const gsl_rng_type * T;
  gsl_rng * r;
  gsl_rng * genstate;

};

#endif /* GEN_WHITENOISE_H_ */
