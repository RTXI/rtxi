//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  File = filtfunc.cpp
//
//

#include <array>
#include <cmath>
#include <complex>

#include "filtfunc.hpp"

#include <gsl/gsl_sf_trig.h>

#include "cmpxpoly.hpp"
#include "poly.hpp"
#include "unwrap.hpp"

FilterTransFunc::FilterTransFunc(int order)
    : Denorm_Cutoff_Freq_Rad(NAN)
    , H_Sub_Zero(NAN)
    , Filter_Order(order)
{
}

std::vector<std::complex<double>> FilterTransFunc::GetPrototypePoles() const
{
  return (prototype_poles);
}

std::vector<std::complex<double>> FilterTransFunc::GetPoles() const
{
  if (Filter_Is_Denormalized) {
    return Denorm_Pole_Locs;
  }
  return prototype_poles;
}

std::complex<double> FilterTransFunc::GetPole(size_t pole_indx) const
{
  if (Filter_Is_Denormalized) {
    return (Denorm_Pole_Locs.at(pole_indx));
  }
  return (prototype_poles.at(pole_indx));
}

std::complex<double> FilterTransFunc::GetZero(size_t zero_indx)
{
  if (Filter_Is_Denormalized) {
    return (Denorm_Zero_Locs.at(zero_indx));
  }
  return (Prototype_Zero_Locs.at(zero_indx));
}

//========================================================
void FilterTransFunc::FrequencyPrewarp(double sampling_interval)
{
  double freq_scale = NAN;
  double warped_analog_cutoff = NAN;
  double desired_digital_cutoff = NAN;

  desired_digital_cutoff = Denorm_Cutoff_Freq_Rad;
  warped_analog_cutoff = (2.0 / sampling_interval)
      * tan(desired_digital_cutoff * sampling_interval / 2.0);
  freq_scale = warped_analog_cutoff / desired_digital_cutoff;

  for (auto& denorm_pole : Denorm_Pole_Locs) {
    denorm_pole *= freq_scale;
  }
  for (auto& denorm_zero : Denorm_Zero_Locs) {
    denorm_zero *= freq_scale;
  }
  const int num = static_cast<int>(Denorm_Pole_Locs.size())
      - static_cast<int>(Denorm_Zero_Locs.size());
  for (int i = 0; i < num; i++) {
    H_Sub_Zero *= freq_scale;
  }
}

//=========================================================
void FilterTransFunc::FilterFrequencyResponse()
{
  std::complex<double> numer;
  std::complex<double> denom;
  std::complex<double> transfer_function;
  std::complex<double> s_val;
  double delta_freq = NAN;
  double magnitude = NAN;
  double phase = NAN;
  double peak_magnitude = NAN;
  std::array<double, 800> mag_resp {};
  std::array<double, 800> phase_resp {};
  std::array<double, 800> group_dly {};

  delta_freq = 0.0125;
  peak_magnitude = -1000.0;

  for (size_t i = 1; i < 800; i++) {
    numer = std::complex<double>(1.0, 0.0);
    denom = std::complex<double>(1.0, 0.0);
    s_val = std::complex<double>(0.0, static_cast<double>(i) * delta_freq);

    for (const auto& denorm_zero : Denorm_Zero_Locs) {
      numer *= (s_val - denorm_zero);
    }

    for (auto denorm_pole : Denorm_Pole_Locs) {
      denom *= (s_val - denorm_pole);
    }

    transfer_function = numer / denom;
    magnitude =
        10.0 * log10(std::abs(transfer_function) * std::abs(transfer_function));
    mag_resp.at(i) = magnitude;
    if (magnitude > peak_magnitude) {
      peak_magnitude = magnitude;
    }
    phase = 180.0 * arg(transfer_function) / M_PI;
    phase_resp.at(i) = phase;
  }
  UnwrapPhase(0, phase_resp.front());
  for (size_t i = 1; i < 800; i++) {
    UnwrapPhase(1, phase_resp.at(i));
  }
  group_dly.at(0) =
      M_PI * (phase_resp.at(0) - phase_resp.at(1)) / (180.0 * delta_freq);
  for (size_t i = 1; i < 800; i++) {
    group_dly.at(i) =
        M_PI * (phase_resp.at(i - 1) - phase_resp.at(i)) / (180.0 * delta_freq);
  }
}

size_t FilterTransFunc::GetNumPoles() const
{
  if (Filter_Is_Denormalized) {
    return Denorm_Pole_Locs.size() + 1;
  }
  return prototype_poles.size() + 1;
}

size_t FilterTransFunc::GetNumZeros() const
{
  if (Filter_Is_Denormalized) {
    return Denorm_Zero_Locs.size() + 1;
  }
  return Prototype_Zero_Locs.size() + 1;
}

std::vector<std::complex<double>> FilterTransFunc::GetPrototypeZeros() const
{
  return Prototype_Zero_Locs;
}

std::vector<std::complex<double>> FilterTransFunc::GetZeros() const
{
  if (Filter_Is_Denormalized) {
    return Denorm_Zero_Locs;
  }
  return Prototype_Zero_Locs;
}

double FilterTransFunc::GetHSubZero() const
{
  return H_Sub_Zero;
}

void FilterTransFunc::SetHSubZero(double hsub)
{
  H_Sub_Zero = hsub;
}

void FilterTransFunc::LowpassDenorm(double cutoff_freq_hz)
{
  Filter_Is_Denormalized = true;
  Denorm_Pole_Locs =
      std::vector<std::complex<double>>(prototype_poles.size());
  Denorm_Zero_Locs =
      std::vector<std::complex<double>>(Prototype_Zero_Locs.size());
  Denorm_Cutoff_Freq_Rad = cutoff_freq_hz;

  for (size_t j = 0; j < Denorm_Pole_Locs.size(); j++) {
    Denorm_Pole_Locs.at(j) = prototype_poles.at(j) * cutoff_freq_hz;
  }
  for (size_t j = 0; j < Denorm_Zero_Locs.size(); j++) {
    Denorm_Zero_Locs.at(j) = Prototype_Zero_Locs.at(j) * cutoff_freq_hz;
  }
  for (size_t j = 0; j < (Denorm_Pole_Locs.size() - Denorm_Zero_Locs.size());
       j++)
  {
    H_Sub_Zero *= cutoff_freq_hz;
  }
}

Polynomial FilterTransFunc::GetDenomPoly() const
{
  //-----------------------------------------------------
  //  if denominator polynomial is not built yet,
  //  build it by multiplying together (s-p.at(i)) binomial
  //  factors where the p.at(i) are the poles of the filter

  if (Denom_Poly.GetDegree() < 0) {
    CmplxPolynomial cmplx_denom_poly {std::complex<double>(1.0, 0.0),
                                      -prototype_poles[1]};
    for (const auto& prototype_pole : prototype_poles) {
      cmplx_denom_poly *=
          CmplxPolynomial {std::complex<double>(1.0, 0.0), -prototype_pole};
    }
    return Polynomial(cmplx_denom_poly);
  }
  return Denom_Poly;
}

Polynomial FilterTransFunc::GetNumerPoly() const
{
  //---------------------------------------------------
  //  if numerator polynomial is not built yet,
  //  build it by multiplying together (s-z.at(i)) binomial
  //  factors where the z.at(i) are the zeros of the filter.

  if (Numer_Poly.GetDegree() < 0) {
    CmplxPolynomial cmplx_poly {std::complex<double>(1.0, 0.0),
                                -Prototype_Zero_Locs[1]};
    for (const auto& prototype_zero : Prototype_Zero_Locs) {
      cmplx_poly *=
          CmplxPolynomial {std::complex<double>(1.0, 0.0), -prototype_zero};
    }
    return Polynomial(cmplx_poly);
  }
  return Numer_Poly;
}

void FilterTransFunc::SetNumerPoly(const Polynomial& poly)
{
  Numer_Poly = poly;
}

void FilterTransFunc::SetDenomPoly(const Polynomial& poly)
{
  Denom_Poly = poly;
}
