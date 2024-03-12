//
// File = abstmath.h
//
#ifndef _ABSTMATH_H_
#define _ABSTMATH_H_

#include <vector>

class PrimeFactorSet
{
public:
  PrimeFactorSet(int num_factors, int* factors);

private:
  int Num_Factors;
  int Num_Distinct_Factors;
  std::vector<int> Factor_Vector;
  std::vector<int> Factor_Multiplicity;
};

class OrderedFactorSet
{
public:
  OrderedFactorSet(int num_factors, const int* factors);

private:
  std::size_t Num_Factors;
  std::vector<int> Factor_Vector;
};

PrimeFactorSet PrimeFactorization(int number);
#endif //_ABSTMATH_H_
