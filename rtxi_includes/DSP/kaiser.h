/*
 * Kaiser window
 */

#ifndef _KAISER_H_
#define _KAISER_H_

#include "gen_win.h"

class KaiserWindow: public GenericWindow {
public:

	// constructors

	KaiserWindow(int num_taps, double Alpha_Parm);

	void GenerateWindow(int num_taps, double Alpha_Parm);

private:

	//int Num_Taps;
	//double *Half_Lag_Window;
	double Alpha_Parm;
};

#endif
