 //
 // File = welcpdgm.cpp
 //                    
 
 #include <stdlib.h>
 #include <string.h>
 #include "complex.h"
 #include "misdefs.h"
 #include "welcpdgm.h"
 #include "fft.h"
 
 WelchPeriodogram::WelchPeriodogram(  
                            SignalSource* signal_source,
                            double samp_intvl,
                            int num_samps_per_seg,
                            int shift_between_segs,
                            int fft_len,
                            GenericWindow* data_wind,
                            int num_segs_to_avg )
                   :PsdEstimate( num_samps_per_seg,
                                 samp_intvl )
 {
  int samp_idx, seg_idx, ovrlap_len;
  complex *time_seg, *ovrlap_seg, *new_seg, *freq_seg;
  complex *windowed_seg;
  double *win_seq;
  double scale_factor;
  
  std::cout << "in WelchPeriodogram" << std::endl;
  
  Psd_Est = new double[fft_len];
  time_seg = new complex[num_samps_per_seg];
  windowed_seg = new complex[num_samps_per_seg];
  freq_seg = new complex[fft_len];

  if( (time_seg ==NULL)||
      (windowed_seg ==NULL)||
      (freq_seg ==NULL) )
    {
     std::cout << "Allocation error in freq_seg in WelchPeriodogram" 
          << std::endl;
     exit(99);
    }
  
  for(samp_idx=0; samp_idx < fft_len; samp_idx++)
    {
     Psd_Est[samp_idx] = 0.0;
    }
  if(data_wind != NULL)
    win_seq = data_wind->GetDataWindow();

  ovrlap_seg = time_seg + shift_between_segs;
  ovrlap_len = num_samps_per_seg-shift_between_segs;
  new_seg = time_seg + ovrlap_len;

  // prefill overlap segment of buffer
  signal_source->GetNextSegment(ovrlap_seg, ovrlap_len);
  for(seg_idx = 0; seg_idx < num_segs_to_avg; seg_idx++)
    {
     // copy overlap samples down to start of buffer
     memmove( time_seg, ovrlap_seg, 
              (sizeof(complex)/sizeof(char))*ovrlap_len);

     // get new samples
     signal_source->GetNextSegment(new_seg, shift_between_segs);

     // apply window sequence to segment of time signal
     if( data_wind == NULL)
       {
       for(samp_idx=0; samp_idx < num_samps_per_seg; samp_idx++)
         {
         windowed_seg[samp_idx] = time_seg[samp_idx];
         }
       }
     else
       {
       for(samp_idx=0; samp_idx < num_samps_per_seg; samp_idx++)
         {
         windowed_seg[samp_idx] = win_seq[samp_idx] * time_seg[samp_idx];
         }
       }
     
     //-----------------------------------
     // compute FFT of windowed segment
     fft(windowed_seg, freq_seg, num_samps_per_seg, fft_len);
     
     for(samp_idx=0; samp_idx < fft_len; samp_idx++)
       {
        Psd_Est[samp_idx] += mag_sqrd(freq_seg[samp_idx]);
       }
    }
  scale_factor = (float)num_segs_to_avg*num_samps_per_seg/samp_intvl;
  for(samp_idx=0; samp_idx < fft_len; samp_idx++)
    {
     Psd_Est[samp_idx] /= scale_factor;
    }  
  delete[] time_seg;
  delete[] freq_seg;
  delete[] windowed_seg;
  return;
 }
 //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
