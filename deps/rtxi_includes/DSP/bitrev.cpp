//
//  File = bitrev.cpp
//

#include "bitrev.h"

int bitrev( int in_val, int num_bits)
 {
  const int lsb_mask=1;
  int bit_pos, work_val, out_val;
 
  work_val = in_val; 
  out_val = 0;
  for (bit_pos=0; bit_pos<num_bits; bit_pos++)
    {
     out_val = (out_val<<1) | (lsb_mask & work_val);
     work_val >>= 1;
    }
  return(out_val);
 }
 //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 
