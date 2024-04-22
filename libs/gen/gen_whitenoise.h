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

#ifndef GEN_WHITENOISE_H_
#define GEN_WHITENOISE_H_

#include <cstdint>
#include <gsl/gsl_randist.h>
#include <gsl/gsl_rng.h>
#include "generator.h"

class GeneratorWNoise : public Generator
{

public:
  explicit GeneratorWNoise(double variance=1.0);
  GeneratorWNoise(const GeneratorWNoise& gen);
  GeneratorWNoise(GeneratorWNoise&& gen) noexcept ;
  GeneratorWNoise& operator=(const GeneratorWNoise&);
  GeneratorWNoise& operator=(GeneratorWNoise&&) noexcept ;
  ~GeneratorWNoise() override;

  void init(uint64_t seed);

  // get readout
  double get() override;

  // set the variance of the generator
  void setVariance(double var);

  double getVariance() const;

private:
  double m_variance;
  uint64_t m_seed = 0;
  const gsl_rng_type* range_type;
  gsl_rng* random_number_generator = nullptr;
};

#endif /* GEN_WHITENOISE_H_ */
