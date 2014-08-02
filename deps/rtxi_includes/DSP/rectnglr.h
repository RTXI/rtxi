/*
 * Rectangular window
 */

#ifndef _RECTNGLR_H_
#define _RECTNGLR_H_

#include "gen_win.h"

class RectangularWindow: public GenericWindow {
public:

	// constructors

	RectangularWindow(int length);

	void GenerateWindow(int length);

private:

	int Num_Taps;
	double *Half_Lag_Window;
};

#endif
