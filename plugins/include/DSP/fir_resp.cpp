/*
 * Functions for FirFilterResponse
 * computes magnitude response
 */

#include <math.h>
#include <stdlib.h>
#include "fir_resp.h"
#include "typedefs.h"
#include "misdefs.h"
#include <complex>
#include <gsl/gsl_math.h>

using namespace std;
//==================================================
//  constructor with all configuration parameters
//  passed in as arguments
//--------------------------------------------------

FirFilterResponse::FirFilterResponse(FirFilterDesign *filter_design, int num_resp_pts, int db_scale_enabled,
		int normalize_enabled, char* resp_file_name) {
	Filter_Design = filter_design;
	Num_Resp_Pts = num_resp_pts;
	Db_Scale_Enabled = db_scale_enabled;
	Normalize_Enabled = normalize_enabled;

	Num_Taps = Filter_Design->GetNumTaps();
	Mag_Resp = new double[Num_Resp_Pts];

	return;
}

//==================================================
//  method to compute magnitude response
//--------------------------------------------------
void FirFilterResponse::ComputeMagResp(void) {
	int resp_indx, tap_indx;
	double lambda;
	complex<double> work (0.0,0.0);

	double* coeff = Filter_Design->GetCoefficients();

	for (resp_indx = 0; resp_indx < Num_Resp_Pts; resp_indx++) {
		lambda = resp_indx * M_PI / (double) Num_Resp_Pts;
		work = complex<double>(0.0, 0.0);

		for (tap_indx = 0; tap_indx < Num_Taps; tap_indx++) {
			work = work + (coeff[tap_indx] * complex<double>(cos(tap_indx * lambda), -sin(tap_indx * lambda)));
		}

		if (Db_Scale_Enabled) {
			Mag_Resp[resp_indx] = 20.0 * log10(norm(work));
		} else {
			Mag_Resp[resp_indx] = norm(work);
		}
	}
	if (Normalize_Enabled)
		NormalizeResponse();

	return;
}

//  method to normalize magnitude response

void FirFilterResponse::NormalizeResponse(void) {
	int n;
	double biggest;

	if (Db_Scale_Enabled) {
		biggest = -100.0;

		for (n = 0; n < Num_Resp_Pts; n++) {
			if (Mag_Resp[n] > biggest)
				biggest = Mag_Resp[n];
		}
		for (n = 0; n < Num_Resp_Pts; n++) {
			Mag_Resp[n] = Mag_Resp[n] - biggest;
		}
	} else {
		biggest = 0.0;

		for (n = 0; n < Num_Resp_Pts; n++) {
			if (Mag_Resp[n] > biggest)
				biggest = Mag_Resp[n];
		}
		for (n = 0; n < Num_Resp_Pts; n++) {
			Mag_Resp[n] = Mag_Resp[n] / biggest;
		}
	}
	return;
}

double* FirFilterResponse::GetMagResp(void) {
	return (Mag_Resp);
}


void FirFilterResponse::DumpMagResp(void) {
	double freq;
	for (int n = 0; n < Num_Resp_Pts; n++) {
		freq = (n * M_PI) / double(Num_Resp_Pts);
	}
	return;
}

double FirFilterResponse::GetIntervalPeak(int nBeg, int nEnd) {
	double peak;
	int n, indexOfPeak;

	peak = -9999.0;
	for (n = nBeg; n < nEnd; n++) {
		if (Mag_Resp[n] > peak) {
			peak = Mag_Resp[n];
			indexOfPeak = n;
		}
	}
	return (peak);
}

