/*
 * Dolph-Chebyshev window
 */

#ifndef _DOLPH_H_
#define _DOLPH_H_

#include "generic_window.hpp"

class DolphChebyWindow : public GenericWindow
{
public:
  // constructors

  DolphChebyWindow(size_t length, double atten);

  void GenerateWindow(size_t length, double Alpha_Parm);

private:
  double Alpha_Parm;
  size_t Interp_Rate;
};

#endif
