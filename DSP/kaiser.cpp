/*
 * Kaiser window
 */

#include <math.h>
#include "kaiser.h"
#include "bessel.cpp"
#include "misdefs.h"
#include <gsl/gsl_math.h>

KaiserWindow::KaiserWindow(int num_taps, double Alpha_Parm) :
	GenericWindow(num_taps) {
	GenerateWindow(num_taps, Alpha_Parm);
}

void KaiserWindow::GenerateWindow(int length, double Alpha_Parm) {
	double denom, x, big_n_sqrd;
	denom = bessel_I_zero(M_PI * Alpha_Parm);
	big_n_sqrd = double(length * length);

	for (int n = 0; n < Half_Length; n++) {
		if (length % 2) // odd length
		{
			x = M_PI * Alpha_Parm * sqrt(1.0 - 4 * n * n / big_n_sqrd);
		} else // even length
		{
			x = M_PI * Alpha_Parm * sqrt(1.0 - (2 * n + 1) * (2 * n + 1) / big_n_sqrd);
		}
		Half_Lag_Win[n] = bessel_I_zero(x) / denom;
	}

	return;
}

