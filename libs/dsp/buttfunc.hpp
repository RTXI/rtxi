//
// File = buttfunc.h
//
#ifndef _BUTTFUNC_H_
#define _BUTTFUNC_H_

#include "filtfunc.hpp"

class ButterworthTransFunc : public FilterTransFunc
{
public:
  // constructor to initialize for a specified filter order
  explicit ButterworthTransFunc(size_t order);
};
#endif
