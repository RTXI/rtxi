 //
 //  file = ex25_02.cpp
 //
 #include <stdlib.h> 
 #include <iostream.h> 
 #include <fstream.h>
 #include <math.h> 
 #include "fft.h"
 #include "misdefs.h"
 #include "uni_rand.h"
 #include "dan_pdgm.h"
 #include "sinawgn.h"
 	
 #ifdef _DEBUG
 ofstream DebugFile("ex25_02.dbg", ios::out);
 #endif

 main()
 { 
  int samp_idx; 
  int num_samps, fft_len;
  double Delta_T, Delta_F;
  complex *time_signal;
  complex *sample_spectrum;
  DaniellPeriodogram *pdgm_dan;   
  long seed, initial_seed, phase_seed, fade_seed, freq_seed;
  double db_value;
  SignalSource* signal_source;

  cout << "Sampling interval?" << endl;
  cin >> Delta_T;

  cout << "samples per segment?" << endl;
  cin >> num_samps;

  cout << "FFT length?" << endl;
  cin >> fft_len;

  Delta_F = 1.0/Delta_T/(float)fft_len;
           
  time_signal = new complex[num_samps];
  sample_spectrum = new complex[fft_len];
  if(sample_spectrum == 0)
    {
     cout << "Error - unable to allocate float_complex[num_samps]"
          << " in main" << endl;
     exit(99);
    }
  
  initial_seed = 4973;  
  phase_seed = 73621;
  fade_seed = 88755;
  freq_seed = 9345;

  signal_source = new SinesInAwgn( Delta_T, cin, cout);

  seed = initial_seed;

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
     
  //-----------------------------------------------------------------------   
  int pp;
  ofstream OutputFile2("dan_db.txt",ios::out);
  
  cout << "Value of P for Daniell periodogram?" << endl;
  cin >> pp;

  pdgm_dan = new DaniellPeriodogram( time_signal,
                                     Delta_T,
                                     num_samps,
                                     fft_len,
									                   NULL,
                                     pp);
                                  
  pdgm_dan->DumpDecibels(&OutputFile2);
  OutputFile2.close();
  //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  ofstream OutputFile2A("dan.txt",ios::out);
  pdgm_dan->DumpNumeric(&OutputFile2A);
  OutputFile2A.close();
  //------------------------------------------------------------------------     

 
  delete [] sample_spectrum;
  return 0;
 }  

