/*
 * Some definitions of math constants and other options for defining filters,
 * some replaced by gsl
 */

#ifndef _MISDEFS_H_
#define _MISDEFS_H_

constexpr double GOLD3 = 0.38196601;
constexpr double GOLD6 = 0.61803399;

constexpr int CONTIN_HALF_LAG = 0;
constexpr int DISCRETE_HALF_LAG = 1;
constexpr int DISCRETE_FULL_LAG = 2;
constexpr int DISCRETE_DATA_WINDOW = 3;

// these defines used in con_resp.cpp
enum class WINDOW_T
{
  RECTANGULAR,
  TRIANGULAR,
  HAMMING,
  HANN,
  DOLPH_CHEBY,
  KAISER
};

constexpr int NO_ZERO_ENDS = 0;
constexpr int ZERO_ENDS = 1;
constexpr int PLOT_CT_WIN = 1;
constexpr int PLOT_CT_MAG_RESP = 2;
constexpr int GEN_DT_WIN_COEFFS = 3;
constexpr int PLOT_DTFT_FOR_DT_WIN = 4;
constexpr int GEN_WINDOWED_FILTER = 5;

#endif
