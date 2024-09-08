//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  File = poly.cpp
//
//  class that implements a polynomial with
//  real-valued coefficients
//

#include "poly.hpp"

#include "cmpxpoly.hpp"

Polynomial::Polynomial(const CmplxPolynomial& original)
{
  std::vector<std::complex<double>> rhs_coefficients;
  original.CopyCoeffs(rhs_coefficients);
}

Polynomial::Polynomial(std::initializer_list<double> coefficients)
    : Coefficients(coefficients)
{
}

Polynomial& Polynomial::operator*=(const Polynomial& right)
{
  //-----------------------------------------------------
  // save pointer to original coefficient array so that
  // this array can be deleted once no longer needed
  std::vector<double> orig_coeff = Coefficients;
  Coefficients.clear();
  Coefficients.resize(orig_coeff.size() + right.Coefficients.size(), 0.0);

  //---------------------------------
  //  perform multiplication

  for (size_t rgt_indx = 0; rgt_indx < right.Coefficients.size(); rgt_indx++) {
    for (size_t orig_indx = 0; orig_indx < orig_coeff.size(); orig_indx++) {
      Coefficients.at(orig_indx + rgt_indx) +=
          (orig_coeff.at(orig_indx) * right.Coefficients.at(rgt_indx));
    }
  }
  return *this;
}

Polynomial& Polynomial::operator/=(const Polynomial& right)
{
  //-----------------------------------------------------
  // save pointer to original coefficient array so that
  // this array can be deleted once no longer needed

  std::vector<double> orig_coeff = Coefficients;
  Coefficients.clear();
  Coefficients.resize(orig_coeff.size() + right.Coefficients.size(), 0.0);

  //---------------------------------
  //  perform multiplication
  for (size_t rgt_indx = 0; rgt_indx < right.Coefficients.size(); rgt_indx++) {
    for (size_t orig_indx = 0; orig_indx < orig_coeff.size(); orig_indx++) {
      Coefficients.at(orig_indx + rgt_indx) +=
          (orig_coeff.at(orig_indx) * right.Coefficients.at(rgt_indx));
    }
  }
  return *this;
}

int Polynomial::GetDegree() const
{
  return static_cast<int>(Coefficients.size()) - 1;
}

double Polynomial::GetCoefficient(size_t k) const
{
  return Coefficients.at(k);
}

