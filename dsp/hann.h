/*
 * Hann window, not to be confused with Hanning window
 */

#ifndef _HANN_H_
#define _HANN_H_

#include "gen_win.h"

class HannWindow: public GenericWindow {
public:

	// constructors

	HannWindow(int length, int zero_ends);
	void GenerateWindow(int length, int zero_ends);

private:

	int Num_Taps;
	double *Half_Lag_Window;
};

#endif
