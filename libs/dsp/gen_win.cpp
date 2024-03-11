/*
 * Generic window for filter
 */

#include <stdexcept>
#include <algorithm>
#include "gen_win.h"

GenericWindow::GenericWindow() = default;

GenericWindow::GenericWindow(int length)
{
  Initialize(length);
}

void GenericWindow::Initialize(int length)
{
  if(length < 0) { throw std::invalid_argument("GenericWindow::Initialize : Negative length given");}
  size_t Half_Length = 0;
  if ((length % 2) != 0) {
    Half_Length = (static_cast<size_t>(length) + 1) / 2;
  } else {
    Half_Length = static_cast<size_t>(length) / 2;
  }

  Half_Lag_Win.resize(Half_Length);
}

double GenericWindow::GetDataWinCoeff(int samp_indx)
{
  if (samp_indx < 0) {
    throw std::invalid_argument(
        "GenericWindow::GetDataWinCoeff : negative index provided");
  }
  size_t middle = 0;
  if ((Length % 2) != 0U) {
    middle = (Length - 1) / 2;
    if (static_cast<size_t>(samp_indx) < middle) {
      return (Half_Lag_Win[middle - static_cast<size_t>(samp_indx)]);
    }
    return (Half_Lag_Win[static_cast<size_t>(samp_indx) - middle]);
  }
  middle = Length / 2;
  if (static_cast<size_t>(samp_indx) < middle) {
    return (Half_Lag_Win[middle - 1 - static_cast<size_t>(samp_indx)]);
  }
  return (Half_Lag_Win[static_cast<size_t>(samp_indx) - middle]);
}

void GenericWindow::NormalizeWindow()
{
  double peak = *std::max_element(Half_Lag_Win.begin(), Half_Lag_Win.end());
  for (double & n : Half_Lag_Win) {
    n /= peak;
  }
}

double* GenericWindow::GetDataWindow()
{
  if (Data_Win.empty()) {
    Data_Win.resize(static_cast<size_t>(GetLength()));
    for (size_t n = 0; n < Length; n++) {
      Data_Win[n] = GetDataWinCoeff(static_cast<int>(n));
    }
  }
  return Data_Win.data();
}

double* GenericWindow::GetHalfLagWindow()
{
  return Half_Lag_Win.data();
}

int GenericWindow::GetNumTaps() const
{
  return static_cast<int>(Length);
}

int GenericWindow::GetHalfLength() const
{
  return static_cast<int>(Half_Length);
}

int GenericWindow::GetLength() const
{
  return static_cast<int>(Length);
}
