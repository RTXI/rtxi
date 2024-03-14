//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  File = abstmath.cpp
//
//  Mathematical Operations for Abstract Algebra
//

#include <cstddef>
#include <vector>

#include "abstmath.h"

#include <math.h>

// NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic)
PrimeFactorSet::PrimeFactorSet(int num_factors, const int* factors)
    : Num_Factors(num_factors)
    , Num_Distinct_Factors(1)
{
  size_t n1 = 0;
  size_t n2 = 0;
  int match = 0;
  int distinct_factor_tally = 0;

  for (n1 = 1; n1 < static_cast<size_t>(num_factors); n1++) {
    match = 0;
    for (n2 = 0; n2 < n1; n2++) {
      // compare factor n1 to all other factors that
      // come before it in the vector factors[].
      // if no match, increment count of distinct factors
      if (factors[n1] == factors[n2]) {
        match = 1;
      }
    }
    if (match == 1) {
      continue;
    }
    Num_Distinct_Factors++;
  }
  Factor_Vector.resize(static_cast<size_t>(Num_Distinct_Factors));
  Factor_Multiplicity.resize(static_cast<size_t>(Num_Distinct_Factors));
  Factor_Vector[0] = factors[0];
  Factor_Multiplicity[0] = 1;
  distinct_factor_tally = 1;
  for (n1 = 1; n1 < static_cast<size_t>(num_factors); n1++) {
    match = 0;
    for (n2 = 0; n2 < static_cast<size_t>(distinct_factor_tally); n2++) {
      if (factors[n1] != Factor_Vector[n2]) {
        continue;
      }
      Factor_Multiplicity[static_cast<size_t>(n2)]++;
      match = 1;
      break;
    }
    if (match != 0) {
      continue;
    }
    Factor_Vector[static_cast<size_t>(distinct_factor_tally)] = factors[n1];
    Factor_Multiplicity[static_cast<size_t>(distinct_factor_tally)] = 1;
    distinct_factor_tally++;
  }
}

OrderedFactorSet::OrderedFactorSet(int num_factors, const int* factors)
    : Num_Factors(static_cast<size_t>(num_factors))
{
  Factor_Vector.resize(Num_Factors);
  for (size_t n = 0; n < static_cast<size_t>(num_factors); n++) {
    Factor_Vector[n] = factors[n];
  }
}

PrimeFactorSet PrimeFactorization(int number)
{
  size_t num_factors = 0;
  size_t first_n = 2;
  int geom_middle = 0;
  int residue = number;
  int factor_not_found = 1;
  std::vector<int> temp_factors(20);

  geom_middle = static_cast<int>(ceil(sqrt(static_cast<double>(residue))));

  for (;;) {
    for (size_t n = first_n; n <= static_cast<size_t>(geom_middle); n++) {
      if (static_cast<size_t>(residue) % n != 0) {
        continue;
      }
      factor_not_found = 0;
      temp_factors[num_factors] = static_cast<int>(n);
      num_factors++;
      residue /= static_cast<int>(n);
      geom_middle = static_cast<int>(ceil(sqrt(static_cast<double>(residue))));
      first_n = n;
      break;
    }
    if (residue == 1) {
      break;
    }
    if (factor_not_found != 0) {
      break;
    }
    factor_not_found = 1;
  }
  if (residue != 1) {
    temp_factors[num_factors] = residue;
    num_factors++;
  }
  PrimeFactorSet factor_set(static_cast<int>(num_factors), temp_factors.data());

  return factor_set;
}
// NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic)
