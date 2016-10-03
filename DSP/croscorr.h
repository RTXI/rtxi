//
// file = croscorr.h
//

#ifndef CROSCORR_H
#define CROSCORR_H

void CrossCorrelation( int seg_len,
                       double* input_segment, 
                       double* output_segment,
                       double* gain,
                       double* phase_delta,
                       int* indx_of_peak);

#endif //CROSCORR_H