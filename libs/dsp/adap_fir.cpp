//
//  File = adap_fir.cpp
//
#include <cmath>
#include <fstream>

#include "adap_fir.h"

#include "dirform1.h"

AdaptiveFir::AdaptiveFir(int num_taps,
                         double* coeff,
                         bool quan_enab,
                         int64_t coeff_quan_factor,
                         int64_t input_quan_factor,
                         int tap_for_trans,
                         int secondary_tap,
                         int transient_len)
    : DirectFormFir(
        num_taps, coeff, quan_enab, coeff_quan_factor, input_quan_factor)
    , Update_Count(0)
    , Trial_Count(0)
    , Tap_For_Trans(static_cast<size_t>(tap_for_trans))
    , Secondary_Tap(static_cast<size_t>(secondary_tap))
    , Transient_Len(static_cast<size_t>(transient_len))
{
  if (transient_len < 0 || tap_for_trans < 0 || secondary_tap < 0) {
    throw std::invalid_argument(
        "AdaptiveFir::AdaptiveFir : Negative value for tap_for_trans, "
        "secondary_tap, or transeint len args");
  }
  Tally_For_Avg.resize(static_cast<size_t>(transient_len));
  Tally_For_Avg_2.resize(static_cast<size_t>(transient_len));
  Sample_Transient.resize(static_cast<size_t>(transient_len));
  Sample_Trans_2.resize(static_cast<size_t>(transient_len));
}

void AdaptiveFir::DumpAvgTransient(int decim_factor)
{
  std::ofstream trans_file("avg_tran.txt", std::ios::out);
  for (size_t samp_idx = 0; samp_idx < Transient_Len;
       samp_idx += static_cast<size_t>(decim_factor))
  {
    trans_file << samp_idx << ", "
               << (Tally_For_Avg[samp_idx] / static_cast<double>(Trial_Count))
               << ", " << Sample_Transient[samp_idx] << "\n";
  }
  trans_file.close();
}

void AdaptiveFir::DumpTrajectory(double min_dist)
{
  double x = NAN;
  double y = NAN;
  double old_x = NAN;
  double old_y = NAN;
  double dist = NAN;
  std::ofstream trans_file("traject.txt", std::ios::out);

  old_x = Sample_Transient[0];
  old_y = Sample_Trans_2[0];
  trans_file << (0) << ", " << old_x << ", " << old_y << "\n";

  for (size_t samp_idx = 1; samp_idx < Transient_Len; samp_idx++) {
    x = Sample_Transient[samp_idx];
    y = Sample_Trans_2[samp_idx];
    dist = sqrt((old_x - x) * (old_x - x) + (old_y - y) * (old_y - y));
    if (dist < min_dist) {
      continue;
    }
    trans_file << samp_idx << ", " << x << ", " << y << '\n';
    old_x = x;
    old_y = y;
  }
  trans_file.close();
}

void AdaptiveFir::DumpAvgTrajectory(double min_dist)
{
  double x = NAN;
  double y = NAN;
  double old_x = NAN;
  double old_y = NAN;
  double dist = NAN;
  std::ofstream trans_file("avg_traj.txt", std::ios::out);

  old_x = Tally_For_Avg[0] / static_cast<double>(Trial_Count);
  old_y = Tally_For_Avg_2[0] / static_cast<double>(Trial_Count);
  trans_file << (0) << ", " << old_x << ", " << old_y << '\n';

  for (size_t samp_idx = 1; samp_idx < Transient_Len; samp_idx++) {
    x = Tally_For_Avg[samp_idx] / static_cast<double>(Trial_Count);
    y = Tally_For_Avg_2[samp_idx] / static_cast<double>(Trial_Count);
    dist = sqrt((old_x - x) * (old_x - x) + (old_y - y) * (old_y - y));
    if (dist < min_dist) {
      continue;
    }
    trans_file << samp_idx << ", " << x << ", " << y << '\n';
    old_x = x;
    old_y = y;
  }
  trans_file.close();
}

void AdaptiveFir::DumpTransient(int decim_factor)
{
  std::ofstream trans_file("samptran.txt", std::ios::out);

  for (size_t samp_idx = 0; samp_idx < Transient_Len;
       samp_idx += static_cast<size_t>(decim_factor))
  {
    trans_file << samp_idx << ", " << Sample_Transient[samp_idx] << '\n';
  }
  trans_file.close();
}

void AdaptiveFir::GetTaps(double* taps)
{
  std::copy(getUnquanCoeff().begin(), getUnquanCoeff().end(), taps);
}
