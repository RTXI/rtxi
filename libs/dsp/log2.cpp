//
//  File = log2.cpp
//

#include "log2.h"

int ilog2(int value_inp)
{
  int log_base_2 = 0;
  int value = 0;

  log_base_2 = 0;
  value = value_inp;

  while (value > 1) {
    value /= 2;
    log_base_2++;
  }

  return log_base_2;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
