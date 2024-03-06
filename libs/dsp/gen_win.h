/*
 * Generic window for filter
 */

#ifndef _GEN_WIN_H_
#define _GEN_WIN_H_

#include <vector>

class GenericWindow
{
public:
  GenericWindow();
  explicit GenericWindow(std::size_t length);

  void Initialize(std::size_t length);

  double GetDataWinCoeff(std::size_t samp_indx);

  void NormalizeWindow();

  std::vector<double>& GetDataWindow();

  std::vector<double>& GetHalfLagWindow();

  std::size_t GetNumTaps();

  std::size_t GetHalfLength();

private:
  std::vector<double> Half_Lag_Win;
  std::vector<double> Lag_Win;
  std::vector<double> Data_Win;
};

#endif
