//
// File = abstmath.h
//
#ifndef _ABSTMATH_H_
#define _ABSTMATH_H_

#include <vector>

class PrimeFactorSet
{
public:
  PrimeFactorSet(const PrimeFactorSet&) = default;
  PrimeFactorSet(PrimeFactorSet&&) = delete;
  PrimeFactorSet& operator=(const PrimeFactorSet&) = default;
  PrimeFactorSet& operator=(PrimeFactorSet&&) = delete;
  // constructor
  PrimeFactorSet(int num_factors, int* factors);
  ~PrimeFactorSet();

private:
  std::vector<int> Factor_Vector;
  std::vector<int> Factor_Multiplicity;
};

class OrderedFactorSet
{
public:
  OrderedFactorSet(const OrderedFactorSet&) = default;
  OrderedFactorSet(OrderedFactorSet&&) = delete;
  OrderedFactorSet& operator=(const OrderedFactorSet&) = default;
  OrderedFactorSet& operator=(OrderedFactorSet&&) = delete;
  // constructor
  OrderedFactorSet(int num_factors, int* factors);
  ~OrderedFactorSet();

private:
  std::vector<int> Factor_Vector;
};

PrimeFactorSet* PrimeFactorization(int number);
#endif //_ABSTMATH_H_
