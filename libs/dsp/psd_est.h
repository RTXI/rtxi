//
//  File = psd_est.h
//
#ifndef _PSD_EST_H_
#define _PSD_EST_H_

#include <fstream>
#include <vector>

#include "fsk_spec.h"

/*!
  * Power Spectrum Distribution (PSD) Estimate
  *
  * Base class for desigining power distribution estimators
  */
class PsdEstimate
{
public:
  /*!
    * Construct a PSD estimator
    *
    * \param num_samps The number of samples for this estimator
    * \param samp_intvl The interval of the estimator
    */
  PsdEstimate(int num_samps, double samp_intvl);

  /*!
    * Dump numerical estimation and theoretical value to output stream
    *
    * \param out_stream Pointer to output stream where to dump data
    * \param ref_spect Pointer to reference theoretical spectrum to dump
    *                  along with the data
    */
  void DumpNumeric(std::ofstream* out_stream, CpfskSpectrum* ref_spect);

  /*!
    * Dump numerical estimation to output stream
    *
    * \param out_stream Pointer to output stream where to dump data
    */
  void DumpNumeric(std::ofstream* out_stream);

  /*!
    * Dump numerical estimation and theoretical value to output stream in Decibels
    *
    * \param out_stream Pointer to output stream where to dump data
    * \param ref_spect Pointer to reference theoretical spectrum to dump
    *                  along with the data
    */
  void DumpDecibels(std::ofstream* out_stream, CpfskSpectrum* ref_spect);

  /*!
    * Dump numerical estimation to output stream in Decibels
    *
    * \param out_stream Pointer to output stream where to dump data
    */
  void DumpDecibels(std::ofstream* out_stream);

protected:
  std::vector<double>& GetPsdEstimate(){ return Psd_Est; }

private:
  int Num_Samps = 0;
  double Delta_F = 0.0;
  double Delta_T = 0.0;
  std::vector<double> Psd_Est;
};

#endif
