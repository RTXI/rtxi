/*
 * Some definitions of math constants and other options for defining filters, some replaced by gsl
 */

#ifndef _MISDEFS_H_
#define _MISDEFS_H_ 

#define PI 3.1415926535897932
//#define PI 3.14159265
const double TWO_PI = 6.2831853071795864;
//#define TWO_PI 6.2831853
#define TRUE 1
#define FALSE 0

#define GOLD3 0.38196601
#define GOLD6 0.61803399
//#define GOLD3 0.38197
//#define GOLD6 0.61803

#define CONTIN_HALF_LAG 0
#define DISCRETE_HALF_LAG 1
#define DISCRETE_FULL_LAG 2
#define DISCRETE_DATA_WINDOW 3

//#define _RECTANGULAR 1
//#define _TRIANGULAR 2
//#define _HAMMING 3
//#define _HANN 4
//#define _DOLPH_CHEBY 5
//#define _KAISER 6

#define _NO_ZERO_ENDS 0
#define _ZERO_ENDS 1

#define _PLOT_CT_WIN 1
#define _PLOT_CT_MAG_RESP 2
#define _GEN_DT_WIN_COEFFS 3
#define _PLOT_DTFT_FOR_DT_WIN 4
#define _GEN_WINDOWED_FILTER 5

#endif
