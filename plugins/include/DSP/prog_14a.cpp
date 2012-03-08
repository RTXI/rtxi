//
//  file = prog_14a.cpp
//
 #include <stdlib.h> 
 #include <iostream.h> 
 #include <fstream.h>
 #include <math.h>
 #include "misdefs.h" 
 #include "fir_resp.h"
 #include "fir_dsgn.h"
 #include "remezalg.h"
   

 #ifdef _DEBUG
 ofstream DebugFile("remez.out", ios::out);
 #endif

 main()
 {
  RemezAlgorithm *remez;
  FirFilterDesign *fir_filter;
  FirFilterResponse *filter_response;
  int filter_length;
  int coeff_indx;
  double ripple_ratio;
  double passband_edge_freq;
  double stopband_edge_freq;
  double *extremal_freqs;
  double *filter_coeffs;
  double *magnitude_response;
  double quant_factor;
  
  read_filter_length:
  cout << "filter length ?\n (must be odd)" << endl;
  cin >> filter_length;
  if((filter_length%2) == 0) goto read_filter_length;
  
  cout << "ripple ratio?" << endl;
  cin >> ripple_ratio;
  
  cout << "passband edge frequency?" << endl;
  cin >> passband_edge_freq;
  
  cout << "stopband edge frequency?" << endl;
  cin >> stopband_edge_freq;
  
  cout << "quantizing factor?\n"
       << " ( 256 for 8 bits, 1024 for 10 bits, etc. )"
       << endl;
  cin >> quant_factor;
    
  extremal_freqs = new double[(filter_length+1)/2];
  filter_coeffs = new double[filter_length];
  
  cout << "ripple_ratio = " << ripple_ratio << endl;
  cout << "passband_edge_freq = " << passband_edge_freq << endl;
  cout << "stopband_edge_freq = " << stopband_edge_freq << endl;
  cout << "filter_length = " << filter_length << endl;
  cout << "quant_factor = " << quant_factor << endl;
  
  remez = new RemezAlgorithm( cin, cout,
                              filter_length,
                              passband_edge_freq,
                              stopband_edge_freq,
                              ripple_ratio,
                              &fir_filter);

  fir_filter->CopyCoefficients(filter_coeffs);  
  
  for( coeff_indx=0; coeff_indx< filter_length; coeff_indx++)
    {
    #ifdef _DEBUG
    DebugFile << "coeff[" << coeff_indx << "] = "
              << filter_coeffs[coeff_indx] << endl;
    #endif
    }

  filter_response = new FirFilterResponse( fir_filter,
                                           cin, cout);
  filter_response->ComputeMagResp();
  filter_response->NormalizeResponse();
  filter_response->DumpMagResp();
  exit(88);
  
  delete [] magnitude_response;
  delete [] extremal_freqs;
  delete [] filter_coeffs;
  delete remez;
  
  return 0;
 }  

