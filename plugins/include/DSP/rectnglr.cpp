/*
 * Rectangular window
 */

#include <math.h>
#include "rectnglr.h"
#include "misdefs.h"
#include <gsl/gsl_math.h>

RectangularWindow::RectangularWindow(int length) {
	Initialize(length);
	GenerateWindow(length);
	return;
}

void RectangularWindow::GenerateWindow(int length) {
	if (length % 2) // odd length window centered at zero
	{
		for (int n = 0; n < Half_Length; n++) {
			Half_Lag_Win[n] = 1.0;
		}
	} else // even length window centered at -1/2
	{
		for (int n = 0; n < Half_Length; n++) {
			Half_Lag_Win[n] = 1.0;
		}
	}
	return;
}
