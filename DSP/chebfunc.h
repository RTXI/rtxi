//
// File = chebfunc.h
//
#ifndef _CHEBFUNC_H_
#define _CHEBFUNC_H_  

#include "filtfunc.h"


class ChebyshevTransFunc : public FilterTransFunc
{
public: 

  // constructor to initialize for a specified filter order 
  ChebyshevTransFunc( int order, 
                      double ripple, 
                      int ripple_bw_norm );

private:
   
}; 
#endif 