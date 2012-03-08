//
// File = polefunc.h
//
#ifndef _POLEFUNC_H_
#define _POLEFUNC_H_  

#include "filtfunc.h"

class AllPoleTransFunc : public FilterTransFunc
{
public: 
  // constructor to initialize for a specified filter order 
  AllPoleTransFunc( std::istream& uin, std::ostream& uout );
}; 
#endif 