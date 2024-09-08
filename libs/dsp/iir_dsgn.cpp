//
//  File = iir_dsgn.cpp
//

#include <cmath>

#include "iir_dsgn.hpp"

IirFilterDesign::IirFilterDesign(size_t num_numer_coeffs,
                                 size_t num_denom_coeffs)
{
  Numer_Coeffs.resize(num_numer_coeffs);
  Denom_Coeffs.resize(num_denom_coeffs + 1);
  Orig_Numer_Coeffs.resize(num_numer_coeffs);
  Orig_Denom_Coeffs.resize(num_denom_coeffs + 1);
}

IirFilterDesign::IirFilterDesign(size_t num_numer_coeffs,
                                 size_t num_denom_coeffs,
                                 std::vector<double> numer_coeffs,
                                 std::vector<double> denom_coeffs)
{
  Numer_Coeffs.resize(num_numer_coeffs);
  Denom_Coeffs.resize(num_denom_coeffs + 1);
  Orig_Numer_Coeffs.resize(num_numer_coeffs);
  Orig_Denom_Coeffs.resize(num_denom_coeffs + 1);

  for (size_t n = 0; n < num_numer_coeffs; n++) {
    Numer_Coeffs.at(n) = numer_coeffs.at(n);
    Orig_Numer_Coeffs.at(n) = numer_coeffs.at(n);
  }
  if (num_numer_coeffs == 0 || num_denom_coeffs == 0) {
    return;
  }
  Denom_Coeffs[0] = 0.0;
  Orig_Denom_Coeffs[0] = 0.0;
  for (size_t n = 1; n <= num_denom_coeffs; n++) {
    Denom_Coeffs.at(n) = denom_coeffs.at(n);
    Orig_Denom_Coeffs.at(n) = denom_coeffs.at(n);
  }
}

void IirFilterDesign::Initialize(size_t num_numer_coeffs,
                                 size_t num_denom_coeffs)
{
  Numer_Coeffs.resize(num_numer_coeffs);
  Denom_Coeffs.resize(num_denom_coeffs + 1);
  Orig_Numer_Coeffs.resize(num_numer_coeffs);
  Orig_Denom_Coeffs.resize(num_denom_coeffs + 1);
}

void IirFilterDesign::QuantizeCoefficients(int quant_factor,
                                           bool rounding_enabled)
{
  int64_t work_long = NAN;
  //-----------------------------------
  // if quant_factor == 0, then restore
  // coefficients to their original,
  // unquantized values
  if (quant_factor == 0) {
    for (size_t n = 0; n < Numer_Coeffs.size(); n++) {
      Numer_Coeffs.at(n) = Orig_Numer_Coeffs.at(n);
    }
    for (size_t n = 1; n <= Denom_Coeffs.size(); n++) {
      Denom_Coeffs.at(n) = Orig_Denom_Coeffs.at(n);
    }
  }

  //-------------------------------------------
  // quantize the original coefficient values
  for (size_t n = 0; n < Numer_Coeffs.size(); n++) {
    if (rounding_enabled) {
      work_long =
          static_cast<int64_t>(quant_factor * Orig_Numer_Coeffs.at(n)) + 0.5;
    } else {
      work_long = static_cast<int64_t>(quant_factor * Orig_Numer_Coeffs.at(n));
    }

    Numer_Coeffs.at(n) =
        static_cast<double>(work_long) / static_cast<double>(quant_factor);
  }
  for (size_t n = 1; n <= Denom_Coeffs.size(); n++) {
    if (rounding_enabled) {
      work_long =
          static_cast<int64_t>((quant_factor * Orig_Denom_Coeffs.at(n)) + 0.5);
    } else {
      work_long = static_cast<int64_t>(quant_factor * Orig_Denom_Coeffs.at(n));
    }

    Denom_Coeffs.at(n) =
        static_cast<double>(work_long) / static_cast<double>(quant_factor);
  }
}

void IirFilterDesign::ScaleCoefficients(double scale_factor)
{
  for (size_t n = 0; n < Numer_Coeffs.size(); n++) {
    Orig_Numer_Coeffs.at(n) = scale_factor * Orig_Numer_Coeffs.at(n);
    Numer_Coeffs.at(n) = Orig_Numer_Coeffs.at(n);
  }
  for (size_t n = 1; n <= Denom_Coeffs.size(); n++) {
    Orig_Denom_Coeffs.at(n) = scale_factor * Orig_Denom_Coeffs.at(n);
    Denom_Coeffs.at(n) = Orig_Denom_Coeffs.at(n);
  }
}

void IirFilterDesign::CopyCoefficients(std::vector<double>& numer_coeff,
                                       std::vector<double>& denom_coeff) const
{
  numer_coeff = Numer_Coeffs;
  denom_coeff = Denom_Coeffs; 
}

std::vector<double> IirFilterDesign::GetNumerCoefficients() const
{
  return Numer_Coeffs;
}

std::vector<double> IirFilterDesign::GetDenomCoefficients() const
{
  return Denom_Coeffs;
}

void IirFilterDesign::SetDenomCoefficients(const std::vector<double>& coeffs)
{
  Denom_Coeffs = coeffs;
}

void IirFilterDesign::SetSamplingInterval(double sampling_interval)
{
  Sampling_Interval = sampling_interval;
}

double IirFilterDesign::GetSamplingInterval() const
{
  return Sampling_Interval;
}

size_t IirFilterDesign::GetNumNumerCoeffs()
{
  return Numer_Coeffs.size();
}

size_t IirFilterDesign::GetNumDenomCoeffs()
{
  return Denom_Coeffs.size();
}

