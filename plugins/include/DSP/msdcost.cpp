//
//  file = msdcost.cpp
//
//  computes cost (in terms of arithmetic operations)
//  of multistage decimator implementations
//

#include <stdlib.h> 
#include <iostream> 
#include <fstream>
#include <math.h>

#include "filtmath.h"
#include "msdcost.h"
    
 void MultistageDecimCost( int num_stages,
                           int* stage_decim_rate,
                           double orig_samp_rate,
                           double passband_edge_freq,
                           double passband_ripple,
                           double stopband_ripple,
                           int num_taps_must_be_odd)
 {
  double delta_p, d_sub_inf;
  int decim_factor, stage;
  double samp_rate, stage_output_rate;
  double trans_width, stopband_edge_freq;
  double total_mult_rate;

  int* num_taps = new int[num_stages];
  double* mult_rate = new double[num_stages];

  delta_p = passband_ripple/num_stages;
  d_sub_inf = DSubInf( delta_p, stopband_ripple );
  std::cout << "D_inf is " << d_sub_inf << std::endl;

  decim_factor = 1;
  for(stage=0; stage<num_stages; stage++)
    decim_factor *= stage_decim_rate[stage];

  stopband_edge_freq = orig_samp_rate/decim_factor/2.0;

  samp_rate = orig_samp_rate;
  total_mult_rate = 0.0;
    double f_dp_ds;
    int new_num_taps;
  for(stage=0; stage<num_stages; stage++)
  {
    stage_output_rate = samp_rate/stage_decim_rate[stage];
    trans_width = stage_output_rate - stopband_edge_freq
                  - passband_edge_freq;
    std::cout << "trans_width = " << trans_width << std::endl;

    // new stuff for better estimate
    f_dp_ds = 11.01217 + 0.51244 * log10(delta_p/stopband_ripple);
    new_num_taps = (int)ceil( (d_sub_inf*samp_rate/trans_width) -
                              (f_dp_ds * trans_width/samp_rate) + 1);
    f_dp_ds = (f_dp_ds * trans_width/samp_rate) + 1.0;
    // end of new stuff

    num_taps[stage] = (int)ceil(d_sub_inf*samp_rate/trans_width);
    if((num_taps[stage]%2 != 1)&& num_taps_must_be_odd)
        num_taps[stage]++;

    mult_rate[stage] = num_taps[stage]*stage_output_rate/2.0;
    total_mult_rate += mult_rate[stage];

    std::cout << "num taps for stage " << stage << " = "
         << num_taps[stage] << std::endl;
    std::cout << "new way num taps is " << new_num_taps << std::endl;
    std::cout << "f(dp, ds) term is " << f_dp_ds << std::endl;
    std::cout << "mult rate = " << mult_rate[stage] << std::endl;
    samp_rate = stage_output_rate;
  }
  std::cout << "total mult rate = "<< total_mult_rate << std::endl;
  delete[] num_taps;
  delete[] mult_rate;
  return;
 }  

