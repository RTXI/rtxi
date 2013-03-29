//
//  file = msmrcost.cpp

//  computes cost (in terms of arithmetic operations)
//  of multistage decimator and interpolator implementations
//
#include <iostream> 
#include <fstream>
#include <math.h>

#include "filtmath.h"
#include "msmrcost.h"
    
 void MultistageMultirateCost( int num_stages,
                               int* stage_factor,
                               double highest_samp_rate,
                               double passband_edge_freq,
                               double passband_ripple,
                               double stopband_ripple,
                               int num_taps_must_be_odd)
 {
  double delta_p, d_sub_inf;
  int overall_rate_factor, stage;
  double high_stage_samp_rate, low_stage_samp_rate;
  double trans_width, stopband_edge_freq;
  double total_mult_rate;

  int* num_taps = new int[num_stages];
  double* mult_rate = new double[num_stages];

  delta_p = passband_ripple/num_stages;
  d_sub_inf = DSubInf( delta_p, stopband_ripple );
  std::cout << "D_inf is " << d_sub_inf << std::endl;

  overall_rate_factor = 1;
  for(stage=0; stage<num_stages; stage++)
    overall_rate_factor *= stage_factor[stage];

  stopband_edge_freq = highest_samp_rate/overall_rate_factor/2.0;

  high_stage_samp_rate = highest_samp_rate;
  total_mult_rate = 0.0;
  for(stage=0; stage<num_stages; stage++)
  {
    low_stage_samp_rate = high_stage_samp_rate
                          /stage_factor[stage];
    trans_width = low_stage_samp_rate - stopband_edge_freq
                  - passband_edge_freq;
    std::cout << "trans_width = " << trans_width << std::endl;
    num_taps[stage] = 
         (int)ceil(d_sub_inf*high_stage_samp_rate/trans_width);
    if((num_taps[stage]%2 != 1)&& num_taps_must_be_odd)
        num_taps[stage]++;

    mult_rate[stage] = num_taps[stage]*low_stage_samp_rate/2.0;
    total_mult_rate += mult_rate[stage];

    std::cout << "num taps for stage " << stage << " = "
         << num_taps[stage] << std::endl;
    std::cout << "mult rate = " << mult_rate[stage] << std::endl;
    high_stage_samp_rate = low_stage_samp_rate;
  }
  std::cout << "total mult rate = "<< total_mult_rate << std::endl;
  delete[] num_taps;
  delete[] mult_rate;
  return;
 }  

