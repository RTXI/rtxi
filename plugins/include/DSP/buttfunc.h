//
// File = buttfunc.h
//
#ifndef _BUTTFUNC_H_
#define _BUTTFUNC_H_  

#include "filtfunc.h"

class ButterworthTransFunc : public FilterTransFunc
{
public: 
  // constructor to initialize for a specified filter order 
  ButterworthTransFunc( int order );
}; 
#endif 