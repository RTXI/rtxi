/*
         The Real-Time eXperiment Interface (RTXI)
         Copyright (C) 2011 Georgia Institute of Technology, University of Utah,
         Weill Cornell Medical College

         Copyright (C) 2021 Michael Bolus

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

#ifndef RTXI_VECTOR_H
#define RTXI_VECTOR_H

#include <vector>

namespace rtxi {

template <typename T = double>
class Vector : public std::vector<T> {
 public:
  using std::vector<T>::vector;
  using std::vector<T>::operator=;
  using std::vector<T>::at;

  explicit Vector(size_t n=1, T fill=0): std::vector<T>(n, fill) {};

  /// conversion: T -> vector
  Vector(T in) : std::vector<T>(1, in) {};
  /// conversion: vector -> double
  operator T() const { return this->at(0); };
};

}  // namespace rtxi

#endif  // RTXI_VECTOR_H
