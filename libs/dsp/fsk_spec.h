//
//  File = fsk_spec.h
//
#ifndef _FSK_SPEC_H_
#define _FSK_SPEC_H_

#include "thy_spec.h"

/*!
 * Continuous-Phase Frequency-Shift Keying Theoretical Spectrum
 */
class CpfskSpectrum : public TheoreticalSpectrum
{
public:
    /*!
    * Initialize the CP FSK Theoretical Spectrum
    *
    * Consult expert or text for variable definitions
    */
  CpfskSpectrum(int big_m, double f_d, double big_t);

    /*!
    * Obtain power spectrum distribution
    *
    * \param freq Frequency at which to get the power spectrum
    *
    * \return The power spectrum at given frequency
    */
  double GetPsdValue(double freq) override;

private:
  int Big_M;
  double Big_T;
  double Freq_Dev;
};
#endif
