//
//  file = ex25_01.cpp
//
 #include <stdlib.h> 
 #include <iostream.h> 
 #include <fstream.h>
 #include <math.h> 
 #include "fft.h"
 #include "misdefs.h"
 #include "uni_rand.h"
 #include "sinawgn.h"
 	
 #ifdef _DEBUG
 ofstream DebugFile("ex25_01.bug", ios::out);
 #endif

 main()
 { 
  int samp_idx; 
  int num_samps, fft_len;
  double Delta_T, Delta_F;
  complex *time_signal;
  double *window_seq;
  complex *sample_spectrum;
  double db_value;
  SignalSource* signal_source;
  complex test_value;

  cout << "Sampling interval?" << endl;
  cin >> Delta_T;

  cout << "samples per segment?" << endl;
  cin >> num_samps;

  cout << "FFT length?" << endl;
  cin >> fft_len;

  window_seq = NULL;

  Delta_F = 1.0/Delta_T/(float)fft_len;
           
  time_signal = new complex[num_samps];
  sample_spectrum = new complex[fft_len];
  
  signal_source = new SinesInAwgn( Delta_T, cin, cout);

  signal_source->GetNextSegment(time_signal, num_samps);
  ofstream TimeFile1("timewave.txt",ios::out);
  for(samp_idx=0; samp_idx<num_samps; samp_idx++)
    {
     TimeFile1 << samp_idx*Delta_T << ",  " 
                 << real(time_signal[samp_idx]) << endl; 
    } 
  TimeFile1.close();
  
  fft(time_signal,sample_spectrum,num_samps,fft_len);

  ofstream OutputFile1("samspcdb.txt",ios::out);
  for(samp_idx=0; samp_idx<fft_len/2; samp_idx++)
    {
     db_value = (float)(10.0 * 
                 log10(double(mag_sqrd(sample_spectrum[samp_idx])/
                 double(num_samps))));
     OutputFile1 << samp_idx*Delta_F << ",  " 
                 << db_value << endl; 
    } 
  OutputFile1.close();
     
  delete [] sample_spectrum;
  return 0;
 }  

