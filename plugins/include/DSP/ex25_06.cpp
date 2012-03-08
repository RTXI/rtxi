//
//  file = ex25_06.cpp
//

 #include <stdlib.h> 
 #include <iostream.h> 
 #include <fstream.h>
 #include <math.h> 
 #include "fft.h"
 #include "misdefs.h"
 #include "uni_rand.h"
 #include "dan_pdgm.h"
 #include "bartpdgm.h"
 #include "welcpdgm.h"
 #include "sinawgn.h"
 #include "gen_win.h"
 #include "hann.h"
 #include "hamming.h"
 #include "trianglr.h"
 	
 #ifdef _DEBUG
 ofstream DebugFile("alias.out", ios::out);
 #endif

 main()
 { 
  int samp_idx; 
  int num_samps, fft_len;
  int windowing_enabled;
  int window_shape;
  double Delta_T, Delta_F;
  complex *time_signal;
  double *window_seq;
  complex *sample_spectrum;
  DaniellPeriodogram *pdgm_dan;   
  BartlettPeriodogram *pdgm_bart;
  WelchPeriodogram *pdgm_welch;   
  long seed, initial_seed, phase_seed, fade_seed, freq_seed;
  double db_value;
  SignalSource* signal_source;
  GenericWindow* data_window;
  complex test_value;

  cout << "Sampling interval?" << endl;
  cin >> Delta_T;

  cout << "samples per segment?" << endl;
  cin >> num_samps;

  cout << "FFT length?" << endl;
  cin >> fft_len;

  cout << "Use windowing? (0 = No, 1 = Yes)" << endl;
  cin >> windowing_enabled;

  if(windowing_enabled)
    {
    cout << "shape of window?\n"
         << _TRIANGULAR << " = triangular\n"
         << _HAMMING << " = Hamming\n"
         << _HANN << " = Hann (hanning, von Hann)\n"
         << endl;
    cin >> window_shape;
    switch(window_shape)
      {
      case _TRIANGULAR:
        data_window = new TriangularWindow( num_samps, _NO_ZERO_ENDS );
        break;
      case _HAMMING:
        data_window = new HammingWindow( num_samps );
        break;
      case _HANN:
        data_window = new HannWindow( num_samps, _NO_ZERO_ENDS );
        break;
      default:
        break;
      }
    //window_seq = new double[num_samps];
    }
  else
    {
    window_seq = NULL;
    data_window = NULL;
    }

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

  if(windowing_enabled)
    {
    window_seq = data_window->GetDataWindow();
    for(samp_idx=0; samp_idx<num_samps; samp_idx++)
       time_signal[samp_idx] *= window_seq[samp_idx];
    }
  
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
  ofstream OutputFile2("dandb.txt",ios::out);
  
  cout << "Value of P for Daniell periodogram?" << endl;
  cin >> pp;

  pdgm_dan = new DaniellPeriodogram( time_signal,
                                     Delta_T,
                                     num_samps,
                                     fft_len,
                                     data_window,
                                     pp);
                                  
  pdgm_dan->DumpDecibels( &OutputFile2 );
  OutputFile2.close();
  //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  ofstream OutputFile2A("dan.txt",ios::out);
  pdgm_dan->DumpNumeric( &OutputFile2A );
  OutputFile2A.close();
  //------------------------------------------------------------------------     
  ofstream OutputFile3("bart_db.txt", ios::out);
  int p_bartlett;
  p_bartlett = 512; 
  seed = initial_seed;

  cout << "Value of P for Bartlett periodogram?" << endl;
  cin >> p_bartlett;

    
  signal_source->ResetSource();
  
  pdgm_bart = new BartlettPeriodogram( signal_source,
                                       Delta_T,
                                       num_samps,
                                       fft_len,
                                       data_window,
                                       p_bartlett);
  
  cout << "got back from BartlettPeriodogram" << endl;

  pdgm_bart->DumpDecibels( &OutputFile3 );
  OutputFile3.close();
  
  //------------------------------------------------------------------------     
  ofstream OutputFile4("welc_db.txt", ios::out);
  int p_welch;
  int welch_shift;
  seed = initial_seed;
  cout << "how many segments for Welch periodogram?" << endl;
  cin >> p_welch;
  cout << "shift from seg to seg (in samples)" << endl;
  cin >> welch_shift;
  
  signal_source->ResetSource( );
  
  pdgm_welch = new WelchPeriodogram( signal_source,
                                     Delta_T,
                                     num_samps,
                                     welch_shift,
                                     fft_len,
                                     data_window,
                                     p_welch);
  
  cout << "got back from WelchPeriodogram" << endl;
    
  pdgm_welch->DumpDecibels( &OutputFile4 );
  OutputFile4.close();
  
 
  //delete [] time_signal;
  delete [] sample_spectrum;
  //delete [] periodogram;  
  return 0;
 }  

