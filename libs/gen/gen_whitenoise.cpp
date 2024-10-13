/*
 * Copyright (C) 2011 Georgia Institute of Technology
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * Generates white noise using the Box-Muller method
 */

#include <utility>
#include <math.h>
#include "gen_whitenoise.h"

GeneratorWNoise::GeneratorWNoise(double variance)
    : m_variance(variance)
    , range_type(gsl_rng_default)
    , random_number_generator(gsl_rng_alloc(range_type))
{
  gsl_rng_set(random_number_generator, m_seed);
}

GeneratorWNoise::GeneratorWNoise(const GeneratorWNoise& gen)
 : Generator(gen) 
{
  if(this == &gen) { return; }
  m_variance = gen.m_variance;
  m_seed = gen.m_seed;
  range_type = gen.range_type;
  random_number_generator = gsl_rng_clone(gen.random_number_generator); 
}

GeneratorWNoise::GeneratorWNoise(GeneratorWNoise&& gen) noexcept
    : m_variance(1.0)
{
  if(this == &gen) { return; }
  std::swap(m_variance, gen.m_variance);
  std::swap(m_seed, gen.m_seed);
  range_type = gen.range_type;
  std::swap(random_number_generator, gen.random_number_generator);
}

GeneratorWNoise& GeneratorWNoise::operator=(const GeneratorWNoise& gen)
{
  if(this == &gen) { return *this; }
  m_variance = gen.m_variance;
  m_seed = gen.m_seed;
  range_type = gen.range_type;
  random_number_generator = gsl_rng_clone(gen.random_number_generator); 
  return *this;
}

GeneratorWNoise& GeneratorWNoise::operator=(GeneratorWNoise&& gen) noexcept
{
  if(this == &gen) { return *this; }
  std::swap(m_variance, gen.m_variance);
  std::swap(m_seed, gen.m_seed);
  range_type = gen.range_type;
  std::swap(random_number_generator, gen.random_number_generator);
  return *this;
}

GeneratorWNoise::~GeneratorWNoise()
{
  gsl_rng_free(random_number_generator);
}

void GeneratorWNoise::init(uint64_t seed)
{
  gsl_rng_set(random_number_generator, seed);
}

double GeneratorWNoise::get()
{
  return gsl_ran_gaussian(random_number_generator, sqrt(m_variance));
}

void GeneratorWNoise::setVariance(double var)
{
  m_variance = var;
}

double GeneratorWNoise::getVariance() const
{
  return m_variance;
}
