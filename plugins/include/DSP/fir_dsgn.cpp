/*
 * Functions for class FirFilterDesign
 */

#include <math.h>
#include <stdlib.h>
#include "misdefs.h"   
#include "fir_dsgn.h"
#include "fir_resp.h"
#include <gsl/gsl_math.h>

// default constructor

FirFilterDesign::FirFilterDesign(void) {
	return;
}

FirFilterDesign::FirFilterDesign(int num_taps) {
	Num_Taps = num_taps;
	Imp_Resp_Coeff = new double[num_taps];
	Original_Coeff = new double[num_taps];
}

FirFilterDesign::FirFilterDesign(int num_taps, double *imp_resp_coeff) {
	Num_Taps = num_taps;
	Imp_Resp_Coeff = new double[num_taps];
	Original_Coeff = new double[num_taps];

	for (int n = 0; n < num_taps; n++) {
		Imp_Resp_Coeff[n] = imp_resp_coeff[n];
		Original_Coeff[n] = imp_resp_coeff[n];
	}
	return;
}

FirFilterDesign::FirFilterDesign(int num_taps, FIR_SYM_T symmetry, double *input_coeff) {
	int last_left, first_right, n;
	Num_Taps = num_taps;
	Coeff_Symmetry = symmetry;

	Original_Coeff = new double[num_taps];
	Imp_Resp_Coeff = new double[num_taps];
	Quant_Coeff = new long[num_taps];
	if (num_taps % 2) { // odd length
		last_left = (num_taps - 1) / 2;
		first_right = last_left;
	} else { // even length
		last_left = (num_taps - 2) / 2;
		first_right = last_left + 1;
	}

	switch (symmetry) {
	case FIR_SYM_EVEN_LEFT:
		for (n = 0; n <= last_left; n++) {
			Original_Coeff[n] = input_coeff[n];
			Imp_Resp_Coeff[n] = input_coeff[n];
			Quant_Coeff[n] = long(Original_Coeff[n]);
		}
		for (n = first_right; n < num_taps; n++) {
			Original_Coeff[n] = input_coeff[num_taps - n - 1];
			Imp_Resp_Coeff[n] = input_coeff[num_taps - n - 1];
			Quant_Coeff[n] = long(Original_Coeff[n]);
		}
	} // end of switch on symmetry
	return;
}

void FirFilterDesign::Initialize(int num_taps) {
	Num_Taps = num_taps;
	Imp_Resp_Coeff = new double[num_taps];
	Original_Coeff = new double[num_taps];
}

//  method to scale coefficients

void FirFilterDesign::ScaleCoefficients(double scale_factor) {
	int n;
	for (n = 0; n < Num_Taps; n++) {
		Original_Coeff[n] = scale_factor * Original_Coeff[n];
		Imp_Resp_Coeff[n] = Original_Coeff[n];
	}
	return;
}

//  scale coefficients so that magnitude response has unity gain at passband peak

void FirFilterDesign::NormalizeFilter(void) {
	int n;
	double passband_peak;
	FirFilterResponse *temp_response;

	temp_response = new FirFilterResponse(this, 500, 0, 0, NULL);
	passband_peak = temp_response->GetIntervalPeak(0, 499);
	delete temp_response;
	for (n = 0; n < Num_Taps; n++) {
		Imp_Resp_Coeff[n] = Original_Coeff[n] / passband_peak;
	}
	return;
}

void FirFilterDesign::CopyCoefficients(double *coeff) {
	for (int n = 0; n < Num_Taps; n++) {
		coeff[n] = Imp_Resp_Coeff[n];
	}
	return;
}

int FirFilterDesign::GetNumTaps(void) {
	return (Num_Taps);
}

double* FirFilterDesign::GetCoefficients(void) {
	// cout << "in fs_dsgn, Imp_Resp_Coeff = " << (void*)Imp_Resp_Coeff << endl;
	return (Imp_Resp_Coeff);
}

void FirFilterDesign::ApplyWindow(GenericWindow *window) {
	for (int n = 0; n < Num_Taps; n++) {
		Imp_Resp_Coeff[n] *= window->GetDataWinCoeff(n);
		Original_Coeff[n] = Imp_Resp_Coeff[n];
	}
}

void FirFilterDesign::ExtractPolyphaseSet(double *coeff, int decim_rate, int rho) {
	double *local_coeff;
	local_coeff = coeff;

	for (int n = rho; n < Num_Taps; n += decim_rate) {
		*local_coeff++ = Imp_Resp_Coeff[n];
	}
	return;
}

