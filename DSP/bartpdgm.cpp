 //
 // File = bartpdgm.cpp
 //                    
 
 #include <stdlib.h>
 #include "complex.h"
 #include "bartpdgm.h"
 #include "fft.h"
 
 BartlettPeriodogram::BartlettPeriodogram(  
                               SignalSource* signal_source,
                               double samp_intvl,
                               int num_samps_per_seg,
                               int fft_len,
                               GenericWindow* data_wind,
                               int num_segs_to_avg )
                     :PsdEstimate( num_samps_per_seg,
                                   samp_intvl )
 {
  int samp_idx, seg_idx;
  double *window_seq;
  complex *time_seg, *freq_seg;
  double scale_factor;
  
  time_seg = new complex[num_samps_per_seg];
  freq_seg = new complex[fft_len];

  if( (freq_seg == NULL)||
      (time_seg==NULL))
    {
     std::cout << "Allocation error in BartlettPeriodogram" << std::endl;
     exit(99);
    }
  
  for(samp_idx=0; samp_idx < fft_len; samp_idx++)
    {
     Psd_Est[samp_idx] = 0.0;
    }
  if(data_wind != NULL)
      window_seq = data_wind->GetDataWindow();
  for(seg_idx = 0; seg_idx < num_segs_to_avg; seg_idx++)
    {
     signal_source->GetNextSegment(time_seg,num_samps_per_seg);
     if(data_wind != NULL)
       {
       for(samp_idx=0; samp_idx<num_samps_per_seg; samp_idx++)
         time_seg[samp_idx] *= window_seq[samp_idx];
       }
     
     fft(time_seg, freq_seg, num_samps_per_seg, fft_len );
     
     for(samp_idx=0; samp_idx < fft_len; samp_idx++)
       {
        Psd_Est[samp_idx] += mag_sqrd(freq_seg[samp_idx]);
       }
    }
  scale_factor = (float)num_segs_to_avg*num_samps_per_seg
                 /samp_intvl;
  for(samp_idx=0; samp_idx < fft_len; samp_idx++)
    {
     Psd_Est[samp_idx] /= scale_factor;
    } 
  delete[] time_seg;
  delete[] freq_seg;
  return;
 }
 //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
