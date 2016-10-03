/*
 * Functions for class FirFilterDesign
 */

#ifndef _FIR_DSGN_H_
#define _FIR_DSGN_H_ 

#include "gen_win.h"
#include "typedefs.h"

typedef enum {
	FIR_SYM_NONE, FIR_SYM_EVEN_LEFT, FIR_SYM_EVEN_RIGHT, FIR_SYM_ODD_LEFT, FIR_SYM_ODD_RIGHT
} FIR_SYM_T;

class FirFilterDesign {
public:

	// default constructor

	FirFilterDesign();

	// constructor that allocates array of length num_taps to hold coefficients

	FirFilterDesign(int num_taps);

	// constructor that allocates array of length num_taps and initializes impulse response

	FirFilterDesign(int num_taps, double *imp_resp_coeff);

	// constructor for filter with symmetric impulse response given half the coefficients

	FirFilterDesign(int num_taps, FIR_SYM_T symmetry, double *imp_resp_coeff);

	// allocate coefficient array *Imp_Resp_Coeff after default constructor

	void Initialize(int num_taps);

	//  method to scale coefficients

	void ScaleCoefficients(double scale_factor);

	//  scale coefficients so that magnitude response has unity gain at passband peak

	void NormalizeFilter(void);

	// set impulse response to values in input array

	void CopyCoefficients(double *coeff);

	double* GetCoefficients(void);

	// get number of filter taps

	int GetNumTaps(void);

	// apply discrete-time window to filter coefficients

	void ApplyWindow(GenericWindow *window);

	// extract a subset of coefficient values for defining
	// a polyphase filter

	void ExtractPolyphaseSet(double *coeff, int decim_rate, int rho);

protected:

	int Num_Taps;

	double* Imp_Resp_Coeff;
	double* Original_Coeff;
	long* Quant_Coeff;
	FIR_SYM_T Coeff_Symmetry;
};

#endif
