/*
 * Functions for FirIdealFilter
 * Design ideal lowpass, highpass, bandpass, or bandstop FIR filter
 */

#include <math.h>
#include <stdlib.h>
#include "misdefs.h"
#include "typedefs.h"   
#include "fir_dsgn.h"
#include "firideal.h"
#include <cstdio>
#include <gsl/gsl_math.h>

FirIdealFilter::FirIdealFilter(int num_taps, double lambda1, double lambda2, int filtertype) :
	LinearPhaseFirDesign(num_taps) {
	if (num_taps % 2) {
		DefineFilter(num_taps, lambda1, lambda2, filtertype);
	} else {
		printf("Error in FIR window filter: even number of taps requested.");
	}
}

void FirIdealFilter::DefineFilter(int num_taps, double lambda1, double lambda2, int band_config) {
	if (num_taps % 2) {
		Fir_Type = 1;
		band_config++;
		Band_Config = (BAND_CONFIG_TYPE) band_config;
		lambda1 *= M_PI;
		lambda2 *= M_PI;
		switch (Band_Config) {
		case _LOWPASS_RESP_:
			Ideal_Lowpass(lambda1);
			break;
		case _BANDPASS_RESP_:
			Ideal_Bandpass(lambda1, lambda2);
			break;
		case _HIGHPASS_RESP_:
			Ideal_Highpass(lambda1);
			break;
		case _BANDSTOP_RESP_:
			Ideal_Bandstop(lambda1, lambda2);
			break;
		} // end of switch on Band_Config
	} else {
		printf("Error in FIR window filter: no filter made!");
	}
}

void FirIdealFilter::Ideal_Lowpass(double lambda1) {
	int n, n_max;
	double m;

	if (Num_Taps % 2) {
		n_max = (Num_Taps - 1) / 2;
		Imp_Resp_Coeff[n_max] = lambda1 / M_PI;
	} else {
		n_max = Num_Taps / 2;
	}

	for (n = 0; n < n_max; n++) {
		m = n - (double) (Num_Taps - 1.0) / 2.0;
		Imp_Resp_Coeff[n] = sin(m * lambda1) / (m * M_PI);
		Imp_Resp_Coeff[Num_Taps - 1 - n] = Imp_Resp_Coeff[n];
	}
}

void FirIdealFilter::Ideal_Highpass(double lambda1) {
	int n, n_max;
	double m;

	if (Num_Taps % 2) {
		n_max = (Num_Taps - 1) / 2;
		Imp_Resp_Coeff[n_max] = 1.0 - lambda1 / M_PI;
	} else {
		n_max = Num_Taps / 2;
	}

	for (n = 0; n < n_max; n++) {
		m = n - (double) (Num_Taps - 1.0) / 2.0;
		Imp_Resp_Coeff[n] = -sin(m * lambda1) / (m * M_PI);
		Imp_Resp_Coeff[Num_Taps - 1 - n] = Imp_Resp_Coeff[n];
	}
}

void FirIdealFilter::Ideal_Bandpass(double lambda1, double lambda2) {
	int n, n_max;
	double m;

	if (Num_Taps % 2) {
		n_max = (Num_Taps - 1) / 2;
		Imp_Resp_Coeff[n_max] = (lambda2 - lambda1) / M_PI;
	} else {
		n_max = Num_Taps / 2;
	}

	for (n = 0; n < n_max; n++) {
		m = n - (double) (Num_Taps - 1.0) / 2.0;
		Imp_Resp_Coeff[n] = (sin(m * lambda2) - sin(m * lambda1)) / (m * M_PI);
		Imp_Resp_Coeff[Num_Taps - 1 - n] = Imp_Resp_Coeff[n];
	}
	return;
}

void FirIdealFilter::Ideal_Bandstop(double lambda1, double lambda2) {
	int n, n_max;
	double m;

	if (Num_Taps % 2) {
		n_max = (Num_Taps - 1) / 2;
		Imp_Resp_Coeff[n_max] = 1.0 + (lambda1 - lambda2) / M_PI;
	} else {
		n_max = Num_Taps / 2;
	}

	for (n = 0; n < n_max; n++) {
		m = n - (double) (Num_Taps - 1.0) / 2.0;
		Imp_Resp_Coeff[n] = (sin(m * lambda1) - sin(m * lambda2)) / (m * M_PI);
		Imp_Resp_Coeff[Num_Taps - 1 - n] = Imp_Resp_Coeff[n];
	}
	return;
}

