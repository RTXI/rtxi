//
// File = elipfunc.h
//
#ifndef _ELIPFUNC_H_
#define _ELIPFUNC_H_  

#include "filtfunc.h"


double ipow( double x, int m);

class EllipticalTransFunc : public FilterTransFunc
{
public: 

  // constructor to initialize for a specified filter order 
  EllipticalTransFunc( int order, 
                       double passband_ripple,
                       double stopband_ripple,
                       double passband_edge,
                       double stopband_edge,
                       int upper_summation_limit );

}; 
#endif 