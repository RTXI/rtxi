//
// File = cmpxpoly.h
//
#ifndef _CMPXPOLY_H_
#define _CMPXPOLY_H_

#include <complex>
#include <vector>

// TODO: CmplxPolynomial class is missing implementation of multiplication,
// addition, subtraction, and division of polynomials. Also missing Correct
// remainder handling
class CmplxPolynomial
{
public:
  CmplxPolynomial() = default;
  CmplxPolynomial(std::initializer_list<std::complex<double>> complex_values);
  explicit CmplxPolynomial(const std::vector<std::complex<double>>& coeff);

  //  multiply assign operator
  CmplxPolynomial& operator*=(const CmplxPolynomial& right);

  //  divide assign operator
  CmplxPolynomial& operator/=(const CmplxPolynomial& divisor);

  // return array of polynomial root values
  std::vector<std::complex<double>> GetRoots();

  // reflect root across the unit circle
  void ReflectRoot(size_t root_idx);

  // get degree of polynomial
  int GetDegree() const;

  // return specified coefficient
  std::complex<double> GetCoeff(size_t k);

  // copy internal coefficients to input
  void CopyCoeffs(std::vector<std::complex<double>>& coeff) const;

private:
  // find roots of the polynomial
  void FindRoots();

  // build sum of powers coefficients from roots
  void BuildFromRoots();

  std::vector<std::complex<double>> Coeff;
  std::vector<std::complex<double>> RemCoeff;
  std::vector<std::complex<double>> Root;
};

#endif
