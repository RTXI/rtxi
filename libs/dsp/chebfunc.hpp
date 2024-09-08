//
// File = chebfunc.h
//
#ifndef _CHEBFUNC_H_
#define _CHEBFUNC_H_

#include "filtfunc.hpp"

class ChebyshevTransFunc : public FilterTransFunc
{
public:
  // constructor to initialize for a specified filter order
  ChebyshevTransFunc(size_t order, double ripple, size_t ripple_bw_norm);
};
#endif
