/*
 * Generic window for filter
 */

#include "gen_win.h"

#include <gsl/gsl_math.h>
#include <stdio.h>

GenericWindow::GenericWindow()
    : Length(0)
    , Half_Length(0)
{
}

GenericWindow::GenericWindow(std::size_t length)
{
  if (length < 0) {
    length = 0;
  }
  Initialize(length);
}

void GenericWindow::Initialize(std::size_t length)
{
  if (length < 0) {
    length = 0;
  }
  Length = static_cast<size_t>(length);
  if ((length % 2) != 0) {
    Half_Length = (Length + 1) / 2;
  } else {
    Half_Length = Length / 2;
  }

  Half_Lag_Win.resize(static_cast<size_t>(Half_Length));
}

double GenericWindow::GetDataWinCoeff(std::size_t samp_indx)
{
  size_t middle = 0;

  if ((Length % 2) != 0) {
    middle = (Length - 1) / 2;
    if (samp_indx < middle) {
      return (Half_Lag_Win[middle - samp_indx]);
    }
    return (Half_Lag_Win[samp_indx - middle]);
  }
  middle = Length / 2;
  if (samp_indx < middle) {
    return (Half_Lag_Win[middle - 1 - samp_indx]);
  }
  return (Half_Lag_Win[samp_indx - middle]);
}

void GenericWindow::NormalizeWindow()
{
  const double peak = Half_Lag_Win[0];
  for (size_t n = 0; n < Half_Length; n++) {
    Half_Lag_Win[n] /= peak;
  }
}

double* GenericWindow::GetDataWindow()
{
  Data_Win.clear();
  for (size_t n = 0; n < Length; n++) {
    Data_Win.push_back(GetDataWinCoeff(n));
  }
  return Data_Win.data();
}

double* GenericWindow::GetHalfLagWindow()
{
  return Half_Lag_Win.data();
}

size_t GenericWindow::GetHalfLength() const
{
  return Half_Length;
}
