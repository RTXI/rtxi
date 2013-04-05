#include <stdlib.h> 
#include <iostream> 
#include <fstream>
#include <math.h>
#include "misdefs.h"
#include "croscorr.h"

extern std::ofstream DebugFile;

void CrossCorrelation( int seg_len,
                       double* input_segment, 
                       double* output_segment,
                       double* gain,
                       double* phase_delta,
                       int* indx_of_peak)
{
  double input_peak, output_peak;
  int indx_of_input_peak, indx_of_output_peak;
  int i;

  //-----------------------------------
  // Find peak in each segment to use
  // for coarse alignment

  input_peak = 0.0;
  output_peak = 0.0;
  indx_of_input_peak = 0;
  indx_of_output_peak = 0;

  for(i=int(0.05*seg_len); i<int(0.9*seg_len); i++)
  {
    if(input_segment[i] > input_peak){
      input_peak = input_segment[i];
      indx_of_input_peak = i;
    }
    if(output_segment[i] > output_peak){
      output_peak = output_segment[i];
      indx_of_output_peak = i;
    }
  }
 
  //-----------------------------------------
  // Find first rising zero-crossing.
  // Then find next falling zero crossing
  // followed by next rising zero-crossing.

#ifdef NOT_DEFINED
  for(i=10; i<seg_len; i++){
    if(input_segment[i] > 0.0) continue;
      beg_indx = i;
      break;
  }
  for(i=beg_indx; i<seg_len; i++){
    if(input_segment[i] < 0.0) continue;
      beg_input_cycle = i;
      break;
  }
  for(i=beg_input_cycle; i<seg_len; i++) {
    if(input_segment[i] > input_peak)
    {
      input_peak = input_segment[i];
      indx_of_input_peak = i;
    }
    if(input_segment[i] > 0.0) continue;
      mid_input_cycle = i;
      break;
  }
  for(i=mid_input_cycle; i<seg_len; i++) {
    if(input_segment[i] < 0.0) continue;
      end_input_cycle = i;
      break;
  }

  for(i=10; i<seg_len; i++){
    if(output_segment[i] > 0.0) continue;
      beg_indx = i;
      break;
  }
  for(i=beg_indx; i<seg_len; i++){
    if(output_segment[i] < 0.0) continue;
      beg_output_cycle = i;
      break;
  }
  for(i=beg_output_cycle; i<seg_len; i++) {
    if(output_segment[i] > output_peak)
    {
      output_peak = output_segment[i];
      indx_of_output_peak = i;
    }
    if(output_segment[i] > 0.0) continue;
      mid_output_cycle = i;
      break;
  }
  for(i=mid_output_cycle; i<seg_len; i++) {
    if(output_segment[i] < 0.0) continue;
      end_output_cycle = i;
      break;
  }
#endif
  *gain = output_peak;
  *indx_of_peak = indx_of_output_peak;
#ifdef NOT_DEFINED
  work = 360.0*(beg_output_cycle-beg_input_cycle)/
                 double(end_input_cycle-beg_input_cycle);
  while(work > 180.0) {
    work -= 360.0;
  }
  while(work < -180.0) {
    work += 360.0;
  }
  *phase_delta = work;
#endif
  *phase_delta = 0.0;

  return;
}  

