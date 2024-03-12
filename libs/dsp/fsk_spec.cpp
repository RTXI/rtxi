//
// file = fsk_spec.cpp
//

#include <cmath>

#include "fsk_spec.h"

#include <gsl/gsl_math.h>
#include <gsl/gsl_sf_trig.h>

CpfskSpectrum::CpfskSpectrum(int big_m, double f_d, double big_t)
    : Big_M(big_m)
    , Big_T(big_t)
    , Freq_Dev(f_d)
{
}

double CpfskSpectrum::GetPsdValue(double freq)
{
  double psi = NAN;
  double phi = NAN;
  double alpha_mn = NAN;
  double a_m = NAN;
  double a_n = NAN;
  double b_mn = NAN;
  double x_m = NAN;
  double x_n = NAN;
  double sum = 0.0;
  double sum2 = 0.0;
  int n = 0;
  int m = 0;

  sum = 0.0;
  for (n = 1; n <= Big_M / 2; n++) {
    sum += (gsl_sf_cos(2 * M_PI * Freq_Dev * Big_T * (2 * n - 1)));
  }
  psi = 2.0 * sum / Big_M;

  sum = 0.0;
  sum2 = 0.0;
  for (n = 1; n <= Big_M; n++) {
    x_n = M_PI * Big_T * (freq - (2 * n - 1 - Big_M) * Freq_Dev);
    a_n = gsl_sf_sinc(x_n);
    sum += a_n * a_n;

    for (m = 1; m <= Big_M; m++) {
      x_m = M_PI * Big_T * (freq - (2 * m - 1 - Big_M) * Freq_Dev);
      a_m = gsl_sf_sinc(x_m);

      alpha_mn = 2 * M_PI * Freq_Dev * Big_T * (m + n - 1 - Big_M);

      b_mn = (gsl_sf_cos(2 * M_PI * freq * Big_T - alpha_mn)
              - psi * gsl_sf_cos(alpha_mn))
          / (1.0 + psi * psi - 2 * psi * gsl_sf_cos(2 * M_PI * freq * Big_T));
      sum2 += b_mn * a_n * a_m;
    }  // end of loop over m
  }  // end of loop over n
  phi = Big_T * (sum / 2.0 + (sum2 / Big_M)) / Big_M;
  return phi;
}
