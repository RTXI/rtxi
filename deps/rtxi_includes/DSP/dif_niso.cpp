//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  File = dif_niso.cpp
//
//  Decimation-In-Frequency FFT
//
//  Naturally-ordered Input / Scrambled Output
//

#include <math.h>
#include "complex.h"
#include "log2.h"
#include "misdefs.h"
#include "cbitrev.h"
#include "dif_niso.h"

void FftDifNiso( complex *array,
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

 pts_in_right_dft = fft_size;
 for( stage=1; stage <=log2_size; stage++)
   { 
    pts_in_left_dft = pts_in_right_dft;  // set pts_in_left_dft = N/(2**(stage-1))
    pts_in_right_dft /= 2;               // set pts_in_right_dft = N/(2**stage)
   
    twiddle = complex(1.0, 0.0);
    trig_arg = PI/pts_in_right_dft;  
    w_fact = complex(cos(trig_arg), -sin(trig_arg));
   
    for( bfly_pos =0; bfly_pos < pts_in_right_dft; bfly_pos++)
      {                             
       for( top_node = bfly_pos; top_node<fft_size; top_node += pts_in_left_dft)
         {                              
          bot_node = top_node + pts_in_right_dft;
          temp = array[top_node] + array[bot_node];
          array[bot_node] = (array[top_node] - array[bot_node]) * twiddle;
          array[top_node] = temp;
         }  // end of loop over top_node
        
       twiddle *= w_fact;
      
      } // end of loop over bfly_pos
   } // end of loop over stage

 ComplexBitReverse(array, fft_size);  

 return;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ 
