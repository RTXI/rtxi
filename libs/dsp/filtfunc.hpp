//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  File = filtfunc.h
//
//

#ifndef _FILTFUNC_H_
#define _FILTFUNC_H_

#include <vector>
#include <complex>
#include "poly.hpp"

class FilterTransFunc
{
public:
  FilterTransFunc()=default;
  explicit FilterTransFunc(int order);

  void FilterFrequencyResponse();

  std::vector<std::complex<double>> GetPrototypePoles() const;

  std::vector<std::complex<double>> GetPoles() const;

  std::complex<double> GetPole(size_t pole_indx) const;

  std::vector<std::complex<double>> GetPrototypeZeros() const;

  std::vector<std::complex<double>> GetZeros() const;

  std::complex<double> GetZero(size_t zero_indx);

  void LowpassDenorm(double cutoff_freq_hz);

  size_t GetNumPoles() const;

  size_t GetNumZeros() const;

  double GetHSubZero() const;

  void SetHSubZero(double hsub);

  Polynomial GetDenomPoly() const;

  Polynomial GetNumerPoly() const;

  void FrequencyPrewarp(double sampling_interval);

  void SetNumerPoly(const Polynomial& poly);

  void SetDenomPoly(const Polynomial& poly);
private:
  Polynomial Denom_Poly;
  Polynomial Numer_Poly;
  std::vector<double> A_Biquad_Coef;
  std::vector<double> B_Biquad_Coef;
  std::vector<double> C_Biquad_Coef;
  std::vector<std::complex<double>> prototype_poles;
  std::vector<std::complex<double>> Prototype_Zero_Locs;
  std::vector<std::complex<double>> Denorm_Pole_Locs;
  std::vector<std::complex<double>> Denorm_Zero_Locs;
  double Denorm_Cutoff_Freq_Rad{};
  double H_Sub_Zero{};
  int Filter_Order{};
  bool Filter_Is_Denormalized{};
};

#endif
