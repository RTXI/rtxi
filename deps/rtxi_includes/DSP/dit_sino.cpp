//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  File = dit_sino.cpp
//
//  Decimation-In-Time FFT
//
//  Scrambled Input / Naturally-ordered Output
//

#include <math.h>
#include "misdefs.h"
#include "log2.h"
#include "cbitrev.h"
#include "dit_sino.h"

void IfftDitSino( complex *array,
                  int fft_size)
{
double trig_arg;
int log2_size;
complex twiddle, w_fact;
complex temp;
int pts_in_left_dft, pts_in_right_dft;
int stage, bfly_pos;
int top_node, bot_node;

log2_size = ilog2(fft_size);

ComplexBitReverse(array, fft_size);  

pts_in_right_dft = 1;
for( stage=1; stage <=log2_size; stage++)
  { 
   pts_in_left_dft = pts_in_right_dft;  // set pts_in_left_dft = 2**(stage-1)
   pts_in_right_dft *= 2;               // set pts_in_right_dft = 2**stage
   
   twiddle = complex(1.0, 0.0);
   trig_arg = PI/pts_in_left_dft;  
   w_fact = complex(cos(trig_arg), sin(trig_arg));
   
   for( bfly_pos =0; bfly_pos < pts_in_left_dft; bfly_pos++)
     {                             
      for( top_node = bfly_pos; top_node<fft_size; top_node += pts_in_right_dft)
        {                              
         bot_node = top_node + pts_in_left_dft;
         temp = array[bot_node] * twiddle;
         array[bot_node] = array[top_node] - temp;
         array[top_node] += temp;
        }  // end of loop over top_node
        
      twiddle *= w_fact;
      
     } // end of loop over bfly_pos
  } // end of loop over stage

return;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ 
void FftDitSino( complex *array,
                 int fft_size)
{
double trig_arg;
int log2_size;
complex twiddle, w_fact;
complex temp;
int pts_in_left_dft, pts_in_right_dft;
int stage, bfly_pos;
int top_node, bot_node;

log2_size = ilog2(fft_size);

ComplexBitReverse(array, fft_size);  

pts_in_right_dft = 1;
for( stage=1; stage <=log2_size; stage++)
  { 
   pts_in_left_dft = pts_in_right_dft;  // set pts_in_left_dft = 2**(stage-1)
   pts_in_right_dft *= 2;               // set pts_in_right_dft = 2**stage
   
   twiddle = complex(1.0, 0.0);
   trig_arg = PI/pts_in_left_dft;  
   w_fact = complex(cos(trig_arg), -sin(trig_arg));
   
   for( bfly_pos =0; bfly_pos < pts_in_left_dft; bfly_pos++)
     {                             
      for( top_node = bfly_pos; top_node<fft_size; top_node += pts_in_right_dft)
        {                              
         bot_node = top_node + pts_in_left_dft;
         temp = array[bot_node] * twiddle;
         array[bot_node] = array[top_node] - temp;
         array[top_node] += temp;
        }  // end of loop over top_node
        
      twiddle *= w_fact;
      
     } // end of loop over bfly_pos
  } // end of loop over stage

return;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ 
