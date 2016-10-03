//
// File = bessfunc.h
//
#ifndef _BESSFUNC_H_
#define _BESSFUNC_H_  

#include "filtfunc.h"
#define MAX_BESSEL_ORDER 10

class BesselTransFunc : public FilterTransFunc
{
public: 
  // constructor to initialize for a specified filter order 
  BesselTransFunc( int order,
                   double passband_edge,
                   int norm_for_delay );
}; 
#endif 