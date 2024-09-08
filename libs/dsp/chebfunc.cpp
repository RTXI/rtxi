//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  File = chebfunc.cpp
//
//  Chebyshev Filter Function
//


#include "chebfunc.hpp"

ChebyshevTransFunc::ChebyshevTransFunc(size_t order,
                                       double ripple,
                                       size_t ripple_bw_norm)
    : FilterTransFunc(order)
{
  double x = NAN;
  double epsilon = NAN;
  double gamma = NAN;
  double big_r = NAN;
  double big_a = NAN;
  double sigma_mult = NAN;
  double omega_mult = NAN;
  std::complex<double> work;

  std::vector<std::complex<double>> prototype_poles(order + 1);
  std::vector<std::complex<double>> prototype_zero(1);

  epsilon = sqrt(pow(10.0, (ripple / 10.0)) - 1.0);
  gamma = pow((1 + sqrt(1.0 + epsilon * epsilon)) / epsilon,
              1.0 / static_cast<double>(order));
  if (ripple_bw_norm != 0U) {
    big_r = 1.0;
  } else {
    big_a = std::log((1.0 + sqrt(1.0 - epsilon * epsilon)) / epsilon) / order;
    big_r = (exp(big_a) + exp(-big_a)) / 2.0;
  }

  sigma_mult = ((1.0 / gamma) - gamma) / (2.0 * big_r);

  omega_mult = ((1.0 / gamma) + gamma) / (2.0 * big_r);

  for (size_t k = 1; k <= order; k++) {
    x = M_PI * ((2 * k) - 1) / (2 * order);

    prototype_poles.at(k) =
        std::complex<double>(sigma_mult * sin(x), omega_mult * cos(x));
  }
  //------------------------------------------------
  //  compute gain factor Ho

  work = std::complex<double> {1.0, 0.0};
  for (size_t k = 0; k < order; k++) {
    work *= -prototype_poles.at(k);
  }

  SetHSubZero(work.real());

  if (order % 2 == 0)  // if order is even
  {
    SetHSubZero(GetHSubZero() / sqrt(1.0 + epsilon * epsilon));
  }
}
