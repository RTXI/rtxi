//
//  File = thy_spec.h
//
#ifndef THEORETICAL_SPECTRUM_HPP
#define THEORETICAL_SPECTRUM_HPP

namespace rtxi::dsp {
class TheoreticalSpectrum
{
public:
  TheoreticalSpectrum() = default;
  TheoreticalSpectrum(const TheoreticalSpectrum&) = default;
  TheoreticalSpectrum(TheoreticalSpectrum&&) = default;
  TheoreticalSpectrum& operator=(const TheoreticalSpectrum&) = default;
  TheoreticalSpectrum& operator=(TheoreticalSpectrum&&) = default;
  virtual ~TheoreticalSpectrum() = default;
  virtual double GetPsdValue(double freq) = 0;
};
}  // namespace rtxi::dsp
#endif
