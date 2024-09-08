//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  File = laguerre.cpp
//
//  Laguerre method for finding polynomial roots
//

#include "laguerre.hpp"

#include <cmath>

using complex_v = std::complex<double>;
int LaguerreMethod(const std::vector<std::complex<double>>& coeff,
                   std::complex<double>& root,
                   double epsilon,
                   double epsilon2,
                   int max_iter)
{
  complex_v p_eval;
  complex_v p_prime;
  complex_v p_double_prime;
  complex_v f;
  complex_v f_sqrd;
  complex_v g;
  complex_v radical;
  complex_v f_plus_rad;
  complex_v f_minus_rad;
  complex_v delta_root;
  double old_delta_mag = NAN;
  double root_mag = NAN;
  double error = NAN;
  old_delta_mag = 1000.0;
  const auto order = coeff.size();

  for (size_t iter = 1; iter <= max_iter; iter++) {
    p_double_prime = complex_v(0.0, 0.0);
    p_prime = complex_v(0.0, 0.0);
    p_eval = coeff.back();
    error = std::abs(p_eval);
    root_mag = std::abs(root);

    for (auto j = coeff.size() - 1; j >= 0; j--) {
      p_double_prime = p_prime + root * p_double_prime;
      p_prime = p_eval + root * p_prime;
      p_eval = coeff.at(j) + root * p_eval;
      error = std::abs(p_eval) + root_mag * error;
    }
    error = epsilon2 * error;
    p_double_prime = 2.0 * p_double_prime;

    if (std::abs(p_eval) < error) {
      return (1);
    }
    f = p_prime / p_eval;
    f_sqrd = f * f;
    g = f_sqrd - p_double_prime / p_eval;
    radical = (order - 1) * ((g * order) - f_sqrd);
    radical = sqrt(radical);
    f_plus_rad = f + radical;
    f_minus_rad = f - radical;
    if (std::abs(f_plus_rad) > std::abs(f_minus_rad)) {
      delta_root = complex_v(static_cast<double>(order), 0.0) / f_plus_rad;
    } else {
      delta_root = complex_v(static_cast<double>(order), 0.0) / f_minus_rad;
    }
    root = root - delta_root;
    if ((iter > 6) && (std::abs(delta_root) > old_delta_mag)) {
      return 2;
    }
    if (std::abs(delta_root) < (epsilon * std::abs(root))) {
      return 3;
    }
    old_delta_mag = std::abs(delta_root);
  }

  return -1;
}
