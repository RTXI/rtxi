/*
 * Functions for FirFilterResponse
 * computes magnitude response
 */

#ifndef _FIR_RESP_H_
#define _FIR_RESP_H_

#include "fir_dsgn.h"

class FirFilterResponse {
public:

	// constructor with all configuration parameters

	FirFilterResponse(FirFilterDesign *filter_design, int num_resp_pts, int db_scale_enabled, int normalize_enabled,
			char* resp_file_name);


	//--------------------------------------
	// method to compute magnitude response

	virtual void ComputeMagResp(void);

	//---------------------------------------
	// method to normalize magnitude response

	void NormalizeResponse(void);

	double* GetMagResp(void);

	void DumpMagResp(void);

	double GetIntervalPeak(int beg_indx, int end_indx);

protected:

	FirFilterDesign *Filter_Design;
	int Num_Resp_Pts;
	int Db_Scale_Enabled;
	int Normalize_Enabled;
	int Num_Taps;
	double* Mag_Resp;
	//-----------------------------------
	// stuff below is for "linear phase" filters
	//
	// filter band configuration: 1 = lowpass,  2 = highpass,
	//                            3 = bandpass, 4 = bandstop
	//int Band_Config;
	//int Fir_Type;
	//int N1, N2, N3, N4;

};

#endif
