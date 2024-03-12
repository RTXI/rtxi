//
//  File = thy_spec.h
//
#ifndef _THY_SPEC_H_
#define _THY_SPEC_H_

/*!
 * Interface class describing a theoretical power spectrum.
 */
class TheoreticalSpectrum
{
public:
  TheoreticalSpectrum() = default;
  TheoreticalSpectrum(const TheoreticalSpectrum&) = default;
  TheoreticalSpectrum(TheoreticalSpectrum&&) = delete;
  TheoreticalSpectrum& operator=(const TheoreticalSpectrum&) = default;
  TheoreticalSpectrum& operator=(TheoreticalSpectrum&&) = delete;
  virtual ~TheoreticalSpectrum() = default;

  /*!
    * Get Power Spectrum Distribution at given Frequency
    *
    * \param freq Frequency to retreive power spectrum value
    *
    * \return Power spectrum at given frequency
    */
  virtual double GetPsdValue(double freq) = 0;
};
#endif
