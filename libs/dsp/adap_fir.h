//
//  File = adap_fir.h
//

#ifndef _ADAP_FIR_H_
#define _ADAP_FIR_H_

#include <fstream>
#include <vector>

#include "dirform1.h"

/*!
 * Adaptive filter
 */
class AdaptiveFir : public DirectFormFir
{
public:
  AdaptiveFir(int num_taps,
              double* coeff,
              bool quan_enab,
              int64_t coeff_quan_factor,
              int64_t input_quan_factor,
              int tap_for_trans,
              int secondary_tap,
              int transient_len);
  /*!
   * Dump average transient values to file
   *
   * this function will dump average transient values to a file in local
   * directory called avg_trans.txt
   *
   * \param decim_factor stride for samples
   */
  void DumpAvgTransient(int decim_factor);

  /*!
   * Dump transient values to file
   *
   * this function will dump transient values to a file in local
   * directory called samptran.txt
   *
   * \param decim_factor stride for samples
   */
  void DumpTransient(int decim_factor);

  /*!
   * Dump transient values to file
   *
   * this function will dump average trajectory values to a file in local
   * directory called avg_traj.txt
   *
   * \param min_dist Minimum distance trajectory to dump
   */
  void DumpAvgTrajectory(double min_dist);

  /*!
   * Dump transient values to file
   *
   * this function will dump average trajectory values to a file in local
   * directory called traject.txt
   *
   * \param min_dist Minimum distance trajectory to dump
   */
  void DumpTrajectory(double min_dist);

  /*!
   * Obtain the tap values for this filter filter
   *
   * \param taps Buffer where to store taps.
   */
  void GetTaps(double* taps);

  virtual double UpdateTaps(double true_samp,
                            double estim_samp,
                            bool trans_save_enab) = 0;
  virtual void ResetTaps() = 0;

private:
  size_t Update_Count;
  size_t Trial_Count;
  size_t Tap_For_Trans;
  size_t Secondary_Tap;
  size_t Transient_Len;
  std::vector<double> Tally_For_Avg;
  std::vector<double> Tally_For_Avg_2;
  std::vector<double> Sample_Transient;
  std::vector<double> Sample_Trans_2;
  std::ofstream* Tap_File = nullptr;
};

#endif
