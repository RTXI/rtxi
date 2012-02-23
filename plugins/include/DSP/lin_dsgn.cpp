/*
 * Design of a linear phase filter, based on general filter design
 */

#include "misdefs.h"
#include "typedefs.h"   
#include "fir_dsgn.h"
#include "lin_dsgn.h"
#include <gsl/gsl_math.h>

// default constructor

LinearPhaseFirDesign::LinearPhaseFirDesign() :
	FirFilterDesign() {
}

LinearPhaseFirDesign::LinearPhaseFirDesign(int num_taps) :
	FirFilterDesign(num_taps) {
}

//  lowpass, bandpass, highpass, or bandstop

BAND_CONFIG_TYPE LinearPhaseFirDesign::GetBandConfig(void) {
	return (Band_Config);
}

int LinearPhaseFirDesign::GetFirType(void) {
	return (Fir_Type);
}

