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
 * Generates white noise using the Box-Muller method
 */

#include "gen_whitenoise.h"

// default constructor

GeneratorWNoise::GeneratorWNoise() :
  variance(1)
{
  numsamples = 1;
  wave.clear();
  gsl_rng_env_setup(); // get default generator type and random seed
  T = gsl_rng_default; // mt
  r = gsl_rng_alloc(T); // default seed = 0
  for (int i = 0; i < numsamples; i++)
    {
      wave.push_back(gsl_ran_gaussian(r, sqrt(variance)));
    }
  numsamples = wave.size();
  index = 0;
  genstate = gsl_rng_clone(r);
  gsl_rng_free(r);

}

GeneratorWNoise::GeneratorWNoise(double variance) :
  Generator()
{
  numsamples = 1;
  wave.clear();
  gsl_rng_env_setup(); // get default generator type and random seed
  T = gsl_rng_default; // mt
  r = gsl_rng_alloc(T); // default seed = 0

  for (int i = 0; i < numsamples; i++)
    {
      wave.push_back(gsl_ran_gaussian(r, sqrt(variance)));
    }
  numsamples = wave.size();
  index = 0;
  genstate = gsl_rng_clone(r);
  gsl_rng_free(r);
}

GeneratorWNoise::~GeneratorWNoise()
{
}

void
GeneratorWNoise::init(double variance)
{
  numsamples = 1;
  wave.clear();
  r = gsl_rng_alloc(T); // default seed = 0
  gsl_rng_memcpy(r, genstate);

  for (int i = 0; i < numsamples; i++)
    {
      wave.push_back(gsl_ran_gaussian(r, sqrt(variance)));
    }
  numsamples = wave.size();
  index = 0;
  gsl_rng_memcpy(genstate, r);
  gsl_rng_free(r);
}

double
GeneratorWNoise::get()
{
  double value = wave[index];
  index++;
  if (index >= numsamples)
    {
      index = 0;
      init(variance);
    }
  return value;

}

void
GeneratorWNoise::setVariance(double var)
{
  variance = var;
}

