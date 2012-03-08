//
//  file = dan_pdgm.cpp
//
 #include <stdlib.h>  
 #include <iostream>
 #include "complex.h" 
 #include "dan_pdgm.h"
 #include "fft.h"

 DaniellPeriodogram::DaniellPeriodogram( complex* time_seq, 
                                         double samp_intvl,
                                         int num_samps,
                                         int fft_len,
                                         GenericWindow* window,
                                         int big_p )
                    :PsdEstimate( num_samps,
                                  samp_intvl)
 {
  int k, m;
  double sum;
  double *pdgrm_dan;
  complex *freq_seq, *windowed_seq;
  double scale_factor;
  
  scale_factor = num_samps*(2*big_p+1);
  
  freq_seq = new complex[fft_len];
  windowed_seq = new complex[fft_len];
  pdgrm_dan = new double[fft_len/2];
  fft( time_seq, freq_seq, num_samps, fft_len);
  
  //-----------------------------------------------------------------
  // Compute first P points of periodogram
  
  for( m=0; m<big_p; m++)
    {
     sum = 0.0;
     for( k=m-big_p; k<0; k++)
       {
        sum += mag_sqrd(freq_seq[num_samps + k]);
       }
     for( k=0; k<=(m+big_p); k++)
       {
        sum += mag_sqrd(freq_seq[k]);
       }                          
     Psd_Est[m] = samp_intvl*sum/scale_factor;
    }
    
  //-----------------------------------------------------------------
  //  Compute periodogram points P thru (N/2)-1
  
  for( m=big_p; m<(fft_len/2); m++)
    {
     sum = 0.0;
     for( k=m-big_p; k<=(m+big_p); k++)
       {
        sum += mag_sqrd(freq_seq[k]);
       }
     Psd_Est[m] = samp_intvl*sum/scale_factor;
    }
  delete[] freq_seq;
  return;
 }
 