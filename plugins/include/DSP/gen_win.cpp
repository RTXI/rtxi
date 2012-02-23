/*
 * Generic window for filter
 */

#include <math.h>
#include <stdio.h>
#include "gen_win.h"
#include "misdefs.h"
#include <gsl/gsl_math.h>

GenericWindow::GenericWindow(void) {
	Length = 0;
	Half_Length = 0;
	Half_Lag_Win = NULL;
	Data_Win = NULL;
}

GenericWindow::GenericWindow(int length) {
	Initialize(length);
}

void GenericWindow::Initialize(int length) {
	Length = length;
	if (length % 2) {
		Half_Length = (length + 1) / 2;
	} else {
		Half_Length = length / 2;
	}

	Half_Lag_Win = new double[Half_Length];
	Data_Win = NULL;

	return;
}

double GenericWindow::GetDataWinCoeff(int samp_indx) {
	int middle;

	if (Length % 2) {
		middle = (Length - 1) / 2;
		if (samp_indx < middle) {
			return (Half_Lag_Win[middle - samp_indx]);
		} else {
			return (Half_Lag_Win[samp_indx - middle]);
		}
	} else {
		middle = Length / 2;
		if (samp_indx < middle) {
			return (Half_Lag_Win[middle - 1 - samp_indx]);
		} else {
			return (Half_Lag_Win[samp_indx - middle]);
		}
	}
}

void GenericWindow::NormalizeWindow(void) {
	double peak;
	peak = Half_Lag_Win[0];
	for (int n = 0; n < Half_Length; n++) {
		Half_Lag_Win[n] /= peak;
	}
	return;
}

double* GenericWindow::GetDataWindow(void) {
	if (Data_Win == NULL) {
		Data_Win = new double[Length];
		for (int n = 0; n < Length; n++) {
			Data_Win[n] = GetDataWinCoeff(n);
		}
	}
	return (Data_Win);
}

double* GenericWindow::GetHalfLagWindow(void) {
	return (Half_Lag_Win);
}

int GenericWindow::GetNumTaps(void) {
	return (Length);
}

int GenericWindow::GetHalfLength(void) {
	return (Half_Length);
}
