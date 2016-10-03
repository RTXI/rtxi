//
//  File = log2.cpp
//

#include <stdlib.h>
#include "log2.h"

int
ilog2(int value_inp)
{
  int log_base_2;
  int value;

  log_base_2 = 0;
  value = value_inp;

  while (value > 1)
    {
      value /= 2;
      log_base_2++;
    }
  /*value = 1;
  int i;
  for (i = 0; i < log_base_2+1; i++)
    {
      value *= 2;
    }
  if (value != value_inp)
    log_base_2 = -1;
*/
  return (log_base_2);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ 
