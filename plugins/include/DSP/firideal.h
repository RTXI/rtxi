/*
 * Functions for FirIdealFilter
 * Design ideal lowpass, highpass, bandpass, or bandstop FIR filter
 */

#ifndef _FIRIDEAL_H_
#define _FIRIDEAL_H_ 

#include "typedefs.h"
#include "gen_win.h"
#include "fir_dsgn.h"
#include "lin_dsgn.h"

class FirIdealFilter: public LinearPhaseFirDesign {
public:

	// constructors

	FirIdealFilter(int num_taps, double lambda1, double lambda2, int filtertype);

	void DefineFilter(int num_taps, double lambda1, double lambda2, int band_config);

private:
	void Ideal_Lowpass(double lambda1);
	void Ideal_Highpass(double lambda1);
	void Ideal_Bandpass(double lambda1, double lambda2);
	void Ideal_Bandstop(double lambda1, double lambda2);

protected:
	double lambda1;
	double lambda2;

};

#endif
