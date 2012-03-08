//
//  file = ex27_01.cpp
//

 #include <stdlib.h> 
 #include <iostream.h> 
 #include <fstream.h>
 #include <math.h> 
 #include "complex.h"
 #include "fft.h"
 #include "misdefs.h"
 #include "hamming.h"
 	
 #ifdef _DEBUG
 ofstream DebugFile("ex27_01.dbg", ios::out);
 #endif

 main()
 { 
  int samp_idx, skip_idx; 
  double Delta_T, Delta_F, samp_rate;
  complex *time_signal;
  complex *sample_spectrum;
  complex *log_spectrum, *cepstrum;
  long initial_seed;
  double sum;
  double speech_sample;
  double db_value;
  int decim_fact;
  complex test_value;
  char input_file_name[25];

  initial_seed = 4973;  


  cout << "sampling rate of data in input file?\n" << endl;
  cin >> samp_rate;

  int samp_per_seg;
  cout << "number of data samples in window?" << endl;
  cin >> samp_per_seg;
  int fft_len;
  cout << "FFT length? (data will be zero-padded to fill this length)" << endl;
  cin >> fft_len;

  time_signal = new complex[fft_len];

  sample_spectrum = new complex[fft_len];
  log_spectrum = new complex[fft_len];
  cepstrum = new complex[fft_len];

  sum = 0.0;

  // read in speech segment
  cout << "enter name of speech input file" << endl;
  cin >> input_file_name;
  cout << input_file_name << endl;
  ifstream input_file(input_file_name, ios::in);
  cout << "input decimation factor?" << endl;
  cin >> decim_fact;

  Delta_T = double(decim_fact)/samp_rate;
  Delta_F = 1.0/(fft_len * Delta_T);

  for(samp_idx=0; samp_idx<samp_per_seg; samp_idx++)
    {
    input_file >> speech_sample;
    time_signal[samp_idx] = speech_sample;
    for(skip_idx=1; skip_idx<decim_fact; skip_idx++)
      {
      input_file >> speech_sample;
      }
    }
  ofstream TimeSigFile("time_sig.txt", ios::out);
  for(samp_idx=0; samp_idx<samp_per_seg; samp_idx++)
    {
    TimeSigFile << samp_idx << ", " 
                << real(time_signal[samp_idx]) << endl;
    }
  TimeSigFile.close();
  for(samp_idx=samp_per_seg; samp_idx<fft_len; samp_idx++)
    {
    time_signal[samp_idx] = 0.0;
    }

  // apply window to data
  HammingWindow *window = new HammingWindow(samp_per_seg);
  for(samp_idx=0; samp_idx<samp_per_seg; samp_idx++)
    {
    time_signal[samp_idx] *= (window->GetDataWinCoeff(samp_idx));
    }
 
  fft(time_signal, sample_spectrum, fft_len);

  ofstream SpecFile("sam_spec.txt", ios::out);
  for(samp_idx=0; samp_idx<fft_len; samp_idx++)
    {
    SpecFile << samp_idx*Delta_F << ", " 
             << cabs(sample_spectrum[samp_idx]) << ", "
             << arg(sample_spectrum[samp_idx]) << endl;
    }
  SpecFile.close();
  //----------------------------------------------
  // cepstral analysis insert
  ofstream LogSpecFile("log_spec.txt", ios::out);
  for(samp_idx=0; samp_idx<fft_len; samp_idx++)
    {
    log_spectrum[samp_idx]
           = complex(log10(cabs(sample_spectrum[samp_idx])),0.0);
    LogSpecFile << samp_idx*Delta_F << ", "
                << cabs(log_spectrum[samp_idx]) << endl;
    }

  LogSpecFile.close();
  ifft(log_spectrum, cepstrum, fft_len);
  ofstream CepstrumFile("cepstrum.txt", ios::out);
  for(samp_idx=1; samp_idx<fft_len/2; samp_idx++)
    {
    db_value = 10.0 * 
               log10(double(mag_sqrd(cepstrum[samp_idx])/
               double(samp_per_seg)));
    
	
    CepstrumFile << samp_idx*Delta_T << ", "
                 << (real(cepstrum[samp_idx])/double(samp_per_seg))
                 << endl;

    }
  CepstrumFile.close();

  delete [] sample_spectrum;
  return 0;
 }  

