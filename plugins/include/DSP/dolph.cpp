/*
 * Dolph-Chebyshev window
 */

#include <math.h>
#include <stdlib.h>
#include "dolph.h"
#include "misdefs.h"
#include "acosh.h"
#include <gsl/gsl_math.h>

DolphChebyWindow::DolphChebyWindow(int length, double atten) :
	GenericWindow(length) {
	double Alpha_Parm = cosh(acosh(pow(10.0, atten / 20.0)) / (length - 1));
	GenerateWindow(length, Alpha_Parm);
}

void DolphChebyWindow::GenerateWindow(int length, double Alpha_Parm) {
	double denom, numer, x;
	double sum_re, sum_im;
	int n, k;
	int num_freq_samps, beg_freq_idx, end_freq_idx;
	double* freq_resp;
	int Interp_Rate = 10;

	freq_resp = new double[length * Interp_Rate];

	beg_freq_idx = ((1 - length) * Interp_Rate) / 2;
	end_freq_idx = ((length - 1) * Interp_Rate) / 2;
	num_freq_samps = Interp_Rate * (length - 1) + 1;

	denom = cosh(length * acosh(Alpha_Parm));

	for (n = beg_freq_idx; n <= end_freq_idx; n++) {
		x = Alpha_Parm * cos((M_PI * n) / double(Interp_Rate * length));
		if (x < 1.0) {
			numer = cos(length * acos(x));
		} else {
			numer = cosh(length * acosh(x));
		}
		if (n < 0) {
			freq_resp[n + num_freq_samps] = numer / denom;
		} else {
			freq_resp[n] = numer / denom;
		}
	}

	//  now do inverse DFT

	for (n = 0; n <= (length - 1) / 2; n++) {
		sum_re = 0.0;
		sum_im = 0.0;

		for (k = 0; k < num_freq_samps; k++) {
			sum_re += (freq_resp[k] * cos((M_PI * 2 * k * n) / (double) num_freq_samps));
			sum_im += (freq_resp[k] * sin((M_PI * 2 * k * n) / (double) num_freq_samps));
		}
		Half_Lag_Win[n] = sum_re / (double) num_freq_samps;
	}

	NormalizeWindow();
	delete freq_resp;
	return;
}
