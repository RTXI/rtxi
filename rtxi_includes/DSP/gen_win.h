/*
 * Generic window for filter
 */

#ifndef _GEN_WIN_H_
#define _GEN_WIN_H_ 

class GenericWindow {
public:

	// constructors

	GenericWindow(void);
	GenericWindow(int length);

	void Initialize(int length);

	double GetDataWinCoeff(int samp_indx);

	void NormalizeWindow(void);

	double* GetDataWindow(void);

	double* GetHalfLagWindow(void);

	int GetNumTaps(void);
	int GetHalfLength(void);

protected:

	int Length;
	int Half_Length;
	double *Half_Lag_Win;
	double *Lag_Win;
	double *Data_Win;
};

#endif

