//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  File = cmpxpoly.cpp
//
//  class that implements a polynomial with
//  complex-valued coefficients
//

#include <initializer_list>

#include "cmpxpoly.hpp"

#include "debug.hpp"
#include "laguerre.hpp"

CmplxPolynomial::CmplxPolynomial(const std::vector<std::complex<double>>& coeff)
    : Coeff(coeff)
{
}

CmplxPolynomial::CmplxPolynomial(
    std::initializer_list<std::complex<double>> complex_values)
    : Coeff(complex_values)
{
}

CmplxPolynomial& CmplxPolynomial::operator*=(const CmplxPolynomial& right)
{
  //-----------------------------------------------------
  // save pointer to original coefficient array so that
  // this array can be deleted once no longer needed
  std::vector<std::complex<double>> orig_coeff = Coeff;

  //-------------------------------------------------------
  //  create new longer array to hold the new coefficients
  auto Degree = orig_coeff.size() + right.Coeff.size();
  Coeff = std::vector<std::complex<double>>(Degree + 1);

  //---------------------------------
  //  perform multiplication
  for (size_t rgt_indx = 0; rgt_indx <= right.Coeff.size(); rgt_indx++) {
    for (size_t orig_indx = 0; orig_indx <= orig_coeff.size(); orig_indx++) {
      Coeff.at(orig_indx + rgt_indx) +=
          (orig_coeff.at(orig_indx) * right.Coeff.at(rgt_indx));
    }
  }

  return *this;
}

CmplxPolynomial& CmplxPolynomial::operator/=(const CmplxPolynomial& divisor)
{
  //----------------------------------------------------
  //  In general, polynomial division will produce a
  //  quotient and a remainder.  This routine returns the
  //  quotient as its result.  The remainder will be
  //  stored in a member variable so that it can be
  //  checked or retrived by subsequent calls to the
  //  appropriate member functions.
  //-----------------------------------------------------

  //-------------------------------------------------------
  //  create new array to hold the new coefficients
  auto dvdnd_deg = Coeff.size();
  auto dvsr_deg = divisor.Coeff.size();

  //---------------------------------
  //  perform division
  std::copy(Coeff.begin(), Coeff.end(), RemCoeff.begin());
  for (auto& coefficient : Coeff) {
    coefficient = std::complex<double>(0.0, 0.0);
  }

  for (size_t k = dvdnd_deg - dvsr_deg; k >= 0; k--) {
    Coeff.at(k) = RemCoeff.at(dvsr_deg + k) / divisor.Coeff.at(dvsr_deg);
    for (size_t j = dvsr_deg + k - 1; j >= k; j--)
      RemCoeff.at(j) -= Coeff.at(k) * divisor.Coeff.at(j - k);
  }
  for (size_t j = dvsr_deg; j <= dvdnd_deg; j++)
    RemCoeff.at(j) = std::complex<double>(0.0, 0.0);
  return *this;
}

void CmplxPolynomial::FindRoots()
{
  CmplxPolynomial root_factor;
  CmplxPolynomial work_poly;
  constexpr double epsilon = 0.0000001;
  constexpr double epsilon2 = 1.0e-10;
  constexpr int max_iter = 12;
  int status = 0;

  //------------------------------------------------
  // find coarse locations for roots
  work_poly = CmplxPolynomial(Coeff);

  for (auto& root : Root) {
    root = std::complex<double>(0.0, 0.0);
    status = LaguerreMethod(work_poly.Coeff, root, epsilon, epsilon2, max_iter);
    if (status < 0) {
      ERROR_MSG(
          "CmplxPolynomial::FindRoots : Laguerre method did not converge");
      exit(55);
    }
    root_factor = {std::complex<double>(1.0, 0.0), -root};
    work_poly /= root_factor;
  }
  Root.back() = -(work_poly.GetCoeff(0));

  //------------------------------------------------
  //  polish the roots
  work_poly = CmplxPolynomial(Coeff);
  for (auto& root : Root) {
    status = LaguerreMethod(work_poly.Coeff, root, epsilon, epsilon2, max_iter);
    if (status < 0) {
      std::cout << "Laguerre method did not converge" << std::endl;
      exit(55);
    }
  }
}

std::vector<std::complex<double>> CmplxPolynomial::GetRoots()
{
  if (Root.empty()) {
    this->FindRoots();
  }
  return Root;
}

void CmplxPolynomial::ReflectRoot(size_t root_idx)
{
  // (1) we must first find the roots (with more accuracy than
  // Laguerre alone provides) then (2) replace the root of
  // interest with its reciprocal and (3) reconstitute
  // the sum-of-powers coefficients from the revised set
  // of roots.
  this->FindRoots();
  Root.at(root_idx) = std::complex<double>(1.0, 0.0) / Root.at(root_idx);
  this->BuildFromRoots();
}

int CmplxPolynomial::GetDegree() const
{
  return static_cast<int>(Coeff.size()) - 1;
}

std::complex<double> CmplxPolynomial::GetCoeff(size_t k)
{
  return Coeff.at(k);
}

void CmplxPolynomial::CopyCoeffs(std::vector<std::complex<double>>& coeff) const
{
  coeff = Coeff;
}

// TODO: Implement BuildFromRoots function for complex polynomials
void CmplxPolynomial::BuildFromRoots(){}
