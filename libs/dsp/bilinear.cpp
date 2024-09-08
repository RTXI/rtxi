//
//  File = bilinear.cpp
//

#include <cmath>

#include "bilinear.hpp"

IirFilterDesign BilinearTransf(const FilterTransFunc& analog_filter,
                               double sampling_interval)

{
  size_t max_poles = 0;
  size_t num_poles = 0;
  size_t num_zeros = 0;
  size_t max_coeff = 0;
  size_t num_numer_coeff = 0;
  double h_const = NAN;
  double h_sub_zero = NAN;
  double denom_mu_zero = NAN;
  std::vector<std::complex<double>> pole;
  std::vector<std::complex<double>> zero;
  std::vector<std::complex<double>> mu;
  std::complex<double> alpha;
  std::complex<double> beta;
  std::complex<double> gamma;
  std::complex<double> delta;
  std::complex<double> work;
  std::complex<double> c_two;
  std::vector<double> a;
  std::vector<double> b;

  pole = analog_filter.GetPoles();
  zero = analog_filter.GetZeros();
  h_sub_zero = static_cast<double>(analog_filter.GetHSubZero());
  if (num_poles > num_zeros) {
    max_poles = num_poles;
  } else {
    max_poles = num_zeros;
  }

  //--------------------------------------------
  // allocate and initialize working storage

  mu.resize(max_poles + 1);
  a.resize(max_poles + 1);
  b.resize(max_poles + 1);

  for (size_t j = 0; j <= max_poles; j++) {
    mu.at(j) = std::complex {0.0, 0.0};
    a.at(j) = 0.0;
    b.at(j) = 0.0;
  }

  //-------------------------------------------
  // compute constant gain factor
  h_const = 1.0;
  work = std::complex {1.0, 0.0};
  c_two = std::complex {2.0, 0.0};

  for (size_t n = 1; n <= num_poles; n++) {
    work = work * (c_two - (sampling_interval * pole.at(n)));
    h_const = h_const * sampling_interval;
  }
  h_const = h_sub_zero * h_const / real(work);

  //--------------------------------------------------
  // compute denominator coefficients
  mu.at(0) = std::complex<double> {1.0, 0.0};

  for (size_t n = 1; n <= num_poles; n++) {
    gamma = std::complex<double> {(2.0 / sampling_interval), 0.0} - pole.at(n);
    delta = std::complex<double> {(-2.0 / sampling_interval), 0.0} - pole.at(n);
    for (size_t j = n; j >= 1; j--) {
      mu.at(j) = gamma * mu.at(j) + (delta * mu.at(j - 1));
    }
    mu.at(0) = gamma * mu.at(0);
  }

  denom_mu_zero = real(mu.at(0));
  for (size_t j = 1; j <= num_poles; j++) {
    a.at(j) = -1.0 * real(mu.at(j)) / denom_mu_zero;
  }

  //-----------------------------------------------------
  //  compute numerator coeffcients
  mu.at(0) = std::complex<double> {1.0, 0.0};
  for (size_t n = 1; n <= max_poles; n++) {
    mu.at(n) = std::complex<double> {0.0, 0.0};
  }

  max_coeff = 0;

  //- - - - - - - - - - - - - - - - - - - - -
  //  compute (1+z**(-1)) ** (N-M)
  for (size_t m = 1; m <= (num_poles - num_zeros); m++) {
    max_coeff++;
    for (size_t j = max_coeff; j >= 1; j--) {
      mu.at(j) = mu.at(j) + mu.at(j - 1);
    }
  }
  for (size_t m = 1; m <= num_zeros; m++) {
    max_coeff++;
    alpha = std::complex<double>((2.0 / sampling_interval), 0.0) - zero.at(m);
    beta = std::complex<double>((-2.0 / sampling_interval), 0.0) - zero.at(m);

    for (size_t j = max_coeff; j >= 1; j--) {
      mu.at(j) = alpha * mu.at(j) + (beta * mu.at(j - 1));
    }
    mu.at(0) = alpha * mu.at(0);
  }
  num_numer_coeff = max_coeff + 1;
  for (size_t j = 0; j < num_numer_coeff; j++) {
    b.at(j) = h_sub_zero * real(mu.at(j)) / denom_mu_zero;
  }

  IirFilterDesign iir_filter(num_numer_coeff, num_poles, b, a);
  iir_filter.SetSamplingInterval(sampling_interval);
  return iir_filter;
}
