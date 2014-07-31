/*
 * Hamming window
 */

#ifndef _HAMMING_H_
#define _HAMMING_H_

#include "gen_win.h"

class HammingWindow: public GenericWindow {
public:

	// constructors

	HammingWindow(int length);

	void GenerateWindow(int length);

private:

	int Num_Taps;
	double *Half_Lag_Window;
};

#endif
