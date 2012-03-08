//
//  File = anlg_rcn.cpp
//

#include <math.h>
#include <stdlib.h>
#include <iostream>
#include "misdefs.h" 
#include "sinc.h"  
#include "anlg_rcn.h"

//=========================================
// default constructor
//-----------------------------------------

AnalogReconst::AnalogReconst( double samp_intvl,
                              int analog_interp_rate,
                              int num_signif_sidelobes,
                              int num_analog_samps)
{
  int i;
  Num_Analog_Samps = num_analog_samps;
  Analog_Interp_Rate = analog_interp_rate;
  Interp_Samp_Intvl = samp_intvl/double(analog_interp_rate);
  Delta_Arg = double(PI)/double(analog_interp_rate);
  Curr_Samp = 0;

  Max_Offset = (num_signif_sidelobes+1)*analog_interp_rate;

  Analog_Signal = new double[num_analog_samps];
  for(i=0; i<num_analog_samps; i++)
  {
    Analog_Signal[i] = 0.0;
  }
  return;
} 
//===============================================
AnalogReconst::~AnalogReconst( void )
{
delete Analog_Signal;
}
//-----------------------------------------------

void AnalogReconst::AddSample( double new_samp )
{
  int offset;
  double value;

  Analog_Signal[Curr_Samp] += new_samp;
  for(offset=1; offset <= Max_Offset; offset ++)
  {
    value = new_samp * sinc(double(offset*PI)/
                            double(Analog_Interp_Rate));
    if(Curr_Samp >= offset)
      Analog_Signal[Curr_Samp - offset] += value;
    if((Curr_Samp+offset) < Num_Analog_Samps)
      Analog_Signal[Curr_Samp + offset] += value;
  }
  Curr_Samp += Analog_Interp_Rate;
  return;
}


//==========================================================
//----------------------------------------------------------

void AnalogReconst::DumpResult( ofstream* out_file )
{
  int i;
  double mse;
  mse = 0.0;
  for(i=Max_Offset; i<(Num_Analog_Samps-Max_Offset); i++)
  {
    (*out_file) << i << ", " << Analog_Signal[i] << std::endl;
  }
  return;
} 

//----------------------------------------------------------

void AnalogReconst::CopyResult( double* output_array )
{
  int i;
  double *out_ptr;

  out_ptr = output_array;
  for(i=0; i<Num_Analog_Samps; i++)
  {
    *out_ptr++ = Analog_Signal[i];
  }
  return;
} 

//----------------------------------------------------------

double AnalogReconst::FindPeak( void )
{
  int i;
  double peak_val;

  peak_val = 0.0;
  for(i=Max_Offset; i<(Num_Analog_Samps-Max_Offset); i++)
  {
    if(Analog_Signal[i] > peak_val) peak_val = Analog_Signal[i];
  }
  return(peak_val);
} 
//----------------------------------------------------------

double AnalogReconst::FindPeakMag( void )
{
  int i;
  double peak_val;

  peak_val = 0.0;
  for(i=Max_Offset; i<(Num_Analog_Samps-Max_Offset); i++)
  {
    if(fabs(Analog_Signal[i]) > peak_val) 
                   peak_val = fabs(Analog_Signal[i]);
  }
  return(peak_val);
} 
//------------------------------------------------------------
double AnalogReconst::CosineCorrelate( double norm_freq, 
                                       double phase_offset,
                                       double cycles_per_corr)
{
double sum, auto_sum;
int samps_per_corr, num_holdoff_samps;
int samp_idx;
double lambda;

sum = 0.0;
auto_sum = 0.0;
samps_per_corr = int(cycles_per_corr/
                     (norm_freq * Interp_Samp_Intvl));
num_holdoff_samps = int(1.0/(norm_freq * Interp_Samp_Intvl));
lambda = TWO_PI * norm_freq * Interp_Samp_Intvl;

for( samp_idx=Max_Offset; 
     samp_idx< (Num_Analog_Samps-Max_Offset);
     samp_idx++)
  {
   auto_sum += (cos(lambda*samp_idx + phase_offset)
               *cos(lambda*samp_idx + phase_offset));
   sum += (Analog_Signal[samp_idx]*
          cos(lambda*samp_idx + phase_offset));
  }
return( sum/auto_sum);
}
