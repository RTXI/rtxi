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

#ifndef GEN_ZAP_H_
#define GEN_ZAP_H_

#include "generator.h"

class GeneratorZap : public Generator
{

public:
  // default constructor
  GeneratorZap();
  GeneratorZap(double freq, double freq2, double amplitude, double duration,
               double dt);

  double get() override;

  // initialize waveform
  void init(double freq, double freq2, double amplitude, double duration,
            double dt);

private:
  double m_freq; // Hz
  double m_freq2;
  double m_duration;
  double m_amplitude;
};

#endif /* GEN_ZAP_H_ */
