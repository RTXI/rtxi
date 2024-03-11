//
//  File = psd_est.h
//
#ifndef _PSD_EST_H_
#define _PSD_EST_H_

#include <vector>
#include <fstream>
#include "fsk_spectrum.hpp"

class PsdEstimate
{
public:
  PsdEstimate(int num_samps, double samp_intvl);
  void DumpNumeric(std::ofstream* out_stream, CpfskSpectrum* ref_spect);
  void DumpNumeric(std::ofstream* out_stream);
  void DumpDecibels(std::ofstream* out_stream, CpfskSpectrum* ref_spect);
  void DumpDecibels(std::ofstream* out_stream);

private:
  int Num_Samps;
  double Delta_F;
  double Delta_T;
  std::vector<double> Psd_Est;
};
#endif
