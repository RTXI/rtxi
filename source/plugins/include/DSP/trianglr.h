/*
 * Triangular window
 */

#ifndef _TRIANGLR_H_
#define _TRIANGLR_H_

#include "gen_win.h"

class TriangularWindow: public GenericWindow {
public:

	// constructors


	TriangularWindow(int length, int zero_ends);
	void GenerateWindow(int length, int zero_ends);

	//private:

	//int Num_Taps;
	//double *Half_Lag_Window;
};

#endif
