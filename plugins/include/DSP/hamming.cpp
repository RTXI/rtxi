/*
 * Hamming window
 */

#include <math.h>
#include "hamming.h"
#include "misdefs.h"
#include <gsl/gsl_math.h>

HammingWindow::HammingWindow(int num_taps) :
	GenericWindow(num_taps) {
	GenerateWindow(num_taps);
}

void HammingWindow::GenerateWindow(int length) {
	for (int n = 0; n < Half_Length; n++) {
		if (length % 2) // odd length
		{
			Half_Lag_Win[n] = 0.54 + 0.46 * cos(double(M_PI * 2) * n / (length - 1));
		} else {
			Half_Lag_Win[n] = 0.54 + 0.46 * cos((2 * n + 1) * M_PI / (length - 1));
		}
	}

	return;
}

