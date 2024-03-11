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
  explicit GenericWindow(int length);

  void Initialize(int length);

  double GetDataWinCoeff(int samp_indx);

  void NormalizeWindow();

  double* GetDataWindow();

  double* GetHalfLagWindow();

  int GetNumTaps() const;
  int GetHalfLength() const;
  int GetLength() const;
private:
  std::size_t Length;
  std::size_t Half_Length;
  std::vector<double> Half_Lag_Win;
  std::vector<double> Lag_Win;
  std::vector<double> Data_Win;
};

#endif
