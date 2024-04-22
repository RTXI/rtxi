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

#ifndef GENERATOR_H_
#define GENERATOR_H_

class Generator
{

public:
  Generator() = default;
  Generator(const Generator&) = default;
  Generator(Generator&&) = default;
  Generator& operator=(const Generator&) = default;
  Generator& operator=(Generator&&) = default;
  virtual ~Generator() = default;

  // get readout for random number generator
  virtual double get() = 0;

  double getDeltaTime() const { return dt; }
  void setDeltaTime(double delta) { dt = delta; }
private:
  double dt{};
};

#endif /* GENERATOR_H_ */
