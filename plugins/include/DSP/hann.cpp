/*
 * Hann window, not to be confused with Hanning window
 */

#include <math.h>
#include "hann.h"
#include "misdefs.h"
#include <gsl/gsl_math.h>

HannWindow::HannWindow(int num_taps, int zero_ends) :
	GenericWindow(num_taps) {
	GenerateWindow(num_taps, zero_ends);
}

void HannWindow::GenerateWindow(int length, int zero_ends) {
	for (int n = 0; n < Half_Length; n++) {
		if (length % 2) // odd length
		{
			if (zero_ends) {
				Half_Lag_Win[n] = 0.5 + 0.5 * cos(M_PI * 2 * n / (length - 1));
			} else {
				Half_Lag_Win[n] = 0.5 + 0.5 * cos(M_PI * 2 * n / (length + 1));
			}
		} else {
			if (zero_ends) {
				Half_Lag_Win[n] = 0.5 + 0.5 * cos((2 * n + 1) * M_PI / (length - 1));
			} else {
				Half_Lag_Win[n] = 0.5 + 0.5 * cos((2 * n + 1) * M_PI / (length + 1));
			}
		}
		//    cout << n << "  " << Half_Lag_Win[n] << endl;
	}
	return;
}

