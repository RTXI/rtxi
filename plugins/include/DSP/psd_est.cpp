//
// file = psd_est.cpp
//

#include <stdlib.h>  
#include <iostream>
#include "psd_est.h"

PsdEstimate::PsdEstimate( int num_samps,
                          double samp_intvl )
{
  Num_Samps = num_samps;
  Delta_F = 1.0/samp_intvl/num_samps;
  Delta_T = samp_intvl;
  Psd_Est = new double[Num_Samps];
}
void PsdEstimate::DumpNumeric( ofstream *out_file,
                               CpfskSpectrum *ref_spec )
{
  int samp_idx;
  double freq, value, theor_val;
  for(samp_idx=0; samp_idx<Num_Samps/2; samp_idx++)
    {
     freq = samp_idx*Delta_F;
     theor_val = ref_spec->GetPsdValue(freq);
     value = Psd_Est[samp_idx];
     (*out_file) << freq << ",  " << value 
                  << ", " << theor_val << std::endl; 
    } 
};

void PsdEstimate::DumpNumeric( ofstream *out_file )
{
  int samp_idx;
  double freq, value;
  for(samp_idx=0; samp_idx<Num_Samps/2; samp_idx++)
    {
     freq = samp_idx*Delta_F;
     value = Psd_Est[samp_idx];
     (*out_file) << freq << ",  " << value << std::endl; 
    } 
};

void PsdEstimate::DumpDecibels( ofstream *out_file,
                                CpfskSpectrum *ref_spec )
{
  int samp_idx;
  double freq, value, theor_val;
  for(samp_idx=0; samp_idx<Num_Samps/2; samp_idx++)
    {
     freq = samp_idx*Delta_F;
     theor_val = 10.0 * log10(ref_spec->GetPsdValue(freq));
     value = 10.0 * log10(Psd_Est[samp_idx]);
     (*out_file) << freq << ",  " << value 
                  << ", " << theor_val << std::endl; 
    } 
};
void PsdEstimate::DumpDecibels( ofstream *out_file )
{
  int samp_idx;
  double freq, value;
  for(samp_idx=0; samp_idx<Num_Samps/2; samp_idx++)
    {
     freq = samp_idx*Delta_F;
     value = 10.0 * log10(Psd_Est[samp_idx]);
     (*out_file) << freq << ",  " << value << std::endl; 
    } 
};

