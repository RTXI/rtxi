//
// file = psd_est.cpp
//

#include <cmath>
#include <iostream>

#include "psd_est.h"

#include <gsl/gsl_sf_log.h>

PsdEstimate::PsdEstimate(int num_samps, double samp_intvl)
    : Num_Samps(num_samps)
    , Delta_F(1.0 / samp_intvl / num_samps)
    , Delta_T(samp_intvl)
{
  Psd_Est.resize(static_cast<size_t>(Num_Samps));
}

void PsdEstimate::DumpNumeric(std::ofstream* out_stream,
                              CpfskSpectrum* ref_spect)
{
  double freq = NAN;
  double value = NAN;
  double theor_val = NAN;
  for (size_t samp_idx = 0; samp_idx < static_cast<size_t>(Num_Samps) / 2;
       samp_idx++)
  {
    freq = static_cast<double>(samp_idx) * Delta_F;
    theor_val = ref_spect->GetPsdValue(freq);
    value = Psd_Est[samp_idx];
    (*out_stream) << freq << ",  " << value << ", " << theor_val << "\n";
  }
}

void PsdEstimate::DumpNumeric(std::ofstream* out_stream)
{
  double freq = NAN;
  double value = NAN;
  for (size_t samp_idx = 0; samp_idx < static_cast<size_t>(Num_Samps) / 2;
       samp_idx++)
  {
    freq = static_cast<double>(samp_idx) * Delta_F;
    value = Psd_Est[samp_idx];
    (*out_stream) << freq << ",  " << value << "\n";
  }
}

void PsdEstimate::DumpDecibels(std::ofstream* out_stream,
                               CpfskSpectrum* ref_spect)
{
  double freq = NAN;
  double value = NAN;
  double theor_val = NAN;
  for (size_t samp_idx = 0; samp_idx < static_cast<size_t>(Num_Samps) / 2;
       samp_idx++)
  {
    freq = static_cast<double>(samp_idx) * Delta_F;
    theor_val = 10.0 * gsl_sf_log(ref_spect->GetPsdValue(freq));
    value = 10.0 * gsl_sf_log(Psd_Est[samp_idx]);
    (*out_stream) << freq << ",  " << value << ", " << theor_val << "\n";
  }
}

void PsdEstimate::DumpDecibels(std::ofstream* out_stream)
{
  double freq = NAN;
  double value = NAN;
  for (size_t samp_idx = 0; samp_idx < static_cast<size_t>(Num_Samps) / 2;
       samp_idx++)
  {
    freq = static_cast<double>(samp_idx) * Delta_F;
    value = 10.0 * gsl_sf_log(Psd_Est[samp_idx]);
    (*out_stream) << freq << ",  " << value << "\n";
  }
}
