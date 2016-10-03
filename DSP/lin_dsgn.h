/*
 * Design of a linear phase filter, based on general filter design
 */

#ifndef _LIN_DSGN_H_
#define _LIN_DSGN_H_ 

#include "typedefs.h"

class LinearPhaseFirDesign: public FirFilterDesign {
public:

	// default constructor

	LinearPhaseFirDesign();

	LinearPhaseFirDesign(int num_taps);

	//	lowpass, bandpass, highpass, or bandstop

	BAND_CONFIG_TYPE GetBandConfig(void);

	int GetFirType(void);

protected:

	BAND_CONFIG_TYPE Band_Config;

	int Fir_Type;
};

#endif
