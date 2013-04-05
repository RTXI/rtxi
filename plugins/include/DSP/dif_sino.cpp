//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  File = dif_sino.cpp
//
//  Decimation-In-Frequency FFT
//
//  Scrambled Input / Naturally-ordered Output
//

#include <math.h>
#include "log2.h"
#include "cbitrev.h"
#include "bitrev.h"
#include "misdefs.h"
#include "dif_sino.h"

void FftDifSino( complex *array,
                 int fft_size)
{
 double trig_arg;
 int log2_size;
 complex twiddle;
 complex temp;
 int pts_in_lft_grp, pts_in_rgt_grp;
 int stage, grp_pos, grp_cntr;
 int top_node, bot_node;

 log2_size = ilog2(fft_size);       
 
 ComplexBitReverse(array, fft_size);

 pts_in_rgt_grp = 1;
 for( stage=1; stage <=log2_size; stage++)
   { 
    pts_in_lft_grp = pts_in_rgt_grp;  // set pts_in_lft_grp = 2**(stage-1)
    pts_in_rgt_grp *= 2;              // set pts_in_rgt_grp = 2**stage
    
    grp_cntr = -1;
   
    for( grp_pos =0; grp_pos < fft_size; grp_pos +=pts_in_rgt_grp)
      {
       grp_cntr++;                             
       trig_arg = (TWO_PI*bitrev(grp_cntr, log2_size-1))/fft_size;  
       twiddle = complex(cos(trig_arg), -sin(trig_arg));
       
       for( top_node = grp_pos; top_node< grp_pos+pts_in_lft_grp; 
                                top_node ++)
         {                              
          bot_node = top_node + pts_in_lft_grp;
          temp = array[top_node] + array[bot_node];
          array[bot_node] = (array[top_node] - array[bot_node]) * twiddle;
          array[top_node] = temp;
         }  // end of loop over top_node
        
      } // end of loop over grp_pos
   } // end of loop over stage

 return;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ 
