//
//  file = ex25_03.cpp
//


 #include <stdlib.h> 
 #include <iostream.h> 
 #include <fstream.h>
 #include <math.h> 
 #include "fft.h"
 #include "misdefs.h"
 #include "sam_spec.h"
 #include "dan_pdgm.h"
 #include "fsk_spec.h"
 #include "cpfsk.h"
 	
 #ifdef _DEBUG
 ofstream DebugFile("ex25_03.out", ios::out);
 #endif

 main()
 { 
  int num_samps;
  double Delta_T, Delta_F;
  complex *time_signal;
  complex *sample_spectrum;
  DaniellPeriodogram *pdgm_dan;
  SampleSpectrum *samp_spec;
  long seed, initial_seed;
  double sum;
  SignalSource* signal_source;
  complex test_value;
  CpfskSpectrum *ref_spect;

  initial_seed = 4973;  

  double fsk_freq_dev;
  cout << "frequency deviation for FSK?" << endl;
  cin >> fsk_freq_dev;

  cout << "sampling interval?\n"
       << "( must be < " << (0.5/fsk_freq_dev)
       << " )" << endl;
  cin >> Delta_T;

  int fsk_big_m;
  cout << "FSK alphabet size?" << endl;
  cin >> fsk_big_m;

  double fsk_symb_dur;
  int fsk_samp_per_symb;
  cout << "samples per FSK symbol?" << endl;
  cin >> fsk_samp_per_symb;
  fsk_symb_dur = fsk_samp_per_symb * Delta_T;

  int samp_per_seg;
  cout << "samp per segment?" << endl;
  cin >> samp_per_seg;
  num_samps = samp_per_seg;
  Delta_F = 1.0/Delta_T/(float)num_samps;

  time_signal = (complex*)new double[2*samp_per_seg];

  sample_spectrum = new complex[num_samps];

  ref_spect = new CpfskSpectrum( fsk_big_m, 
                                 fsk_freq_dev, 
                                 fsk_symb_dur );
    
  signal_source = new CpfskSource( Delta_T,
                                   fsk_freq_dev,
                                   fsk_samp_per_symb,
                                   initial_seed,
                                   samp_per_seg,
                                   0);

  seed = initial_seed;
  sum = 0.0;
  signal_source->GetNextSegment(time_signal,samp_per_seg); 

  samp_spec = new SampleSpectrum( time_signal,
                                  Delta_T,
                                  num_samps);

  ofstream OutputFile1("samspcdb.txt",ios::out);
  samp_spec->DumpDecibels( &OutputFile1, ref_spect );
  OutputFile1.close();
     
  ofstream OutputFile1A("samspc.txt",ios::out);
  samp_spec->DumpNumeric( &OutputFile1A, ref_spect );
  OutputFile1A.close();
     
  //-----------------------------------------------------------------------   
  int pp;
  
  cout << "Value of P for Daniell periodogram?" << endl;
  cin >> pp;

  pdgm_dan = new DaniellPeriodogram( time_signal,
                                     Delta_T,
                                     num_samps,
                                     num_samps,
									                   NULL,
                                     pp );
                                  
  ofstream OutputFile2("dan_db.txt",ios::out);
  pdgm_dan->DumpDecibels( &OutputFile2, ref_spect );
  OutputFile2.close();
  //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  ofstream OutputFile2A("dan.txt",ios::out);
  pdgm_dan->DumpNumeric( &OutputFile2A, ref_spect );
  OutputFile2A.close();

  delete [] sample_spectrum;
  return 0;
 }  

