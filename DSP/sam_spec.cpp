//
//  file = sam_spec.cpp
//

 #include <stdlib.h>  
 #include <iostream>
 #include "complex.h" 
 #include "sam_spec.h"
 #include "fft.h"

 SampleSpectrum::SampleSpectrum( complex* time_signal, 
                                 double samp_intvl,
                                 int num_samps )
                :PsdEstimate( num_samps,
                              samp_intvl)
 {
  complex *sample_spectrum = new complex[num_samps];

  fft(time_signal,sample_spectrum,num_samps);
  for(int samp_idx=0; samp_idx<num_samps; samp_idx++)
    {
    Psd_Est[samp_idx] = mag_sqrd(sample_spectrum[samp_idx]);
    }
  return;
 }
 