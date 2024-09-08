//
// File = poly.h
//
#ifndef _POLY_H_
#define _POLY_H_

#include "cmpxpoly.hpp"

// TODO: Figure out if Polynomial class is redundant. Seems like it does everything a
// complex polynomial class does.

// TODO: Define multiplication, division, summation, and subtraction overloads for
// Polynomial class

class Polynomial
{
public:
  Polynomial()=default;
  Polynomial(std::initializer_list<double> coefficients);
  //  conversion constructor
  explicit Polynomial(const CmplxPolynomial& original);

  //  multiply assign operator
  Polynomial& operator*=(const Polynomial& right);

  //  divide assign operator
  Polynomial& operator/=(const Polynomial& right);

  // get degree of polynomial
  int GetDegree() const;

  // return specified coefficient
  double GetCoefficient(size_t k) const;

private:
  std::vector<double> Coefficients;
};
#endif
