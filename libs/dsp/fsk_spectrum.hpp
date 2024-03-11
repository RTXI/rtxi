//
//  File = fsk_spec.h
//
#ifndef FSK_SPECTRUM_HPP
#define FSK_SPECTRUM_HPP

#include "theoretical_spectrum.hpp"

namespace rtxi::dsp {
class CpfskSpectrum : public rtxi::dsp::TheoreticalSpectrum
{
public:
  CpfskSpectrum(int big_m, double f_d, double big_t);
  double GetPsdValue(double freq) override;

private:
  int Big_M;
  double Big_T;
  double Freq_Dev;
};

}  // namespace rtxi::dsp
#endif
