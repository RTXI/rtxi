//
//  File = thy_spec.h
//
#ifndef _THY_SPEC_H_
#define _THY_SPEC_H_

class TheoreticalSpectrum
{
public:
  TheoreticalSpectrum(const TheoreticalSpectrum&) = default;
  TheoreticalSpectrum(TheoreticalSpectrum&&) = delete;
  TheoreticalSpectrum& operator=(const TheoreticalSpectrum&) = default;
  TheoreticalSpectrum& operator=(TheoreticalSpectrum&&) = delete;
  virtual ~TheoreticalSpectrum() = default;
  virtual double GetPsdValue(double freq) = 0;
};
#endif
