/*
 * Dolph-Chebyshev window
 */

#ifndef _DOLPH_H_
#define _DOLPH_H_

#include "gen_win.h"

class DolphChebyWindow : public GenericWindow
{
public:

  // constructors
  
  DolphChebyWindow( int length, double atten);

  void GenerateWindow(int length, double Alpha_Parm );
  
private:
  double Alpha_Parm;
  int Interp_Rate;
  //int Num_Taps;
  //double *Half_Lag_Window;
};

#endif
  
