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

#ifndef GEN_SAW_H_
#define GEN_SAW_H_

#include <array>
#include "generator.h"

class GeneratorSaw : public Generator
{
public:
  // default constructor
  GeneratorSaw();
  GeneratorSaw(double delay, double width, double amplitude, double dt);

  double get() override;

  // initialize waveform
  void init(double delay, double width, double amplitude, double dt);

private:
  double m_delay;
  double m_width;
  double m_amplitude;
  std::array<double, 2> slopes;
  std::array<double, 2> intersects;
};

#endif /* GEN_SAW_H_ */
