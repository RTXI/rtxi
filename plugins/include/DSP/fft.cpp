//
//  File = fft.cpp
//

#include <iostream> 
#include <fstream>

#include "complex.h"
#include "dit_sino.h"
#include "fft.h"

void ifft( complex* sample_spectrum,
           complex* time_signal,
           int num_samps )
{
 int i;
 for(i=0; i<num_samps; i++)
   {
    time_signal[i] = sample_spectrum[i];
   }
 IfftDitSino(time_signal, num_samps);
 return;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ 
void fft( complex* time_signal,
          complex* sample_spectrum,
          int num_samps )
{
 int i;
 for(i=0; i<num_samps; i++)
   {
    sample_spectrum[i] = time_signal[i];
   }
 FftDitSino(sample_spectrum, num_samps);
 return;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ 
void fft( complex* time_signal,
          complex* sample_spectrum,
          int num_samps,
          int fft_len )
{
 int i;
 for(i=0; i<num_samps; i++)
   {
    sample_spectrum[i] = time_signal[i];
   }
 for(i=num_samps; i<fft_len; i++)
   {
    sample_spectrum[i] = complex(0.0,0.0);
   }
 FftDitSino(sample_spectrum, fft_len);
 return;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ 
void fft( complex* signal,
          int num_samps )
{
 FftDitSino(signal, num_samps);
 return;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ 
