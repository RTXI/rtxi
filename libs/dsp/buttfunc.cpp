//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  File = buttfunc.cpp
//
//  Butterworth Filter Response
//

#include <cmath>
#include <cstddef>

#include "buttfunc.hpp"

ButterworthTransFunc::ButterworthTransFunc(size_t order)
    : FilterTransFunc(order)
{
  double x = NAN;

  std::vector<std::complex<double>> prototype_poles(
      static_cast<size_t>(2 * (order + 1)));
  std::vector<std::complex<double>> prototype_zeros(2);

  for (size_t k = 0; k < order; k++) {
    x = M_PI * (order + (2 * k) - 1) / (2 * order);
    prototype_poles.at(k) = std::complex<double> {cos(x), sin(x)};
  }
  SetHSubZero(1.0);
}
