//
//  file = prog_20a.cpp
//

#include <stdlib.h> 
#include <iostream.h> 
#include <fstream.h>
#include <math.h>
#include "misdefs.h"
#include "filtmath.h"
#include "abstmath.h"
#include "msdcost.h"
    

#ifdef _DEBUG
 ofstream DebugFile("multstag.bug", ios::out);
#endif

 main()
 {
  double passband_ripple, stopband_ripple;
  double orig_samp_rate;
  int decim_factor, num_stages, stage;
  double passband_edge_freq, stopband_edge_freq;
  int composite;
  int num_taps_must_be_odd;
  PrimeFactorSet *prime_factors;

  cout << "desired decimation factor?" << endl;
  cin >> decim_factor;

  cout << "total passband ripple?" << endl;
  cin >> passband_ripple;
  
  cout << "total stopband ripple?" << endl;
  cin >> stopband_ripple;

  prime_factors = PrimeFactorization( decim_factor );

  cout << "number of stages?" << endl;
  cin >> num_stages;

  int* stage_decim_rate = new int[num_stages];

  for(;;)
  {
    composite = 1;
    for(stage=0; stage<num_stages; stage++)
    {
      cout << "stage "<< stage << " decimation factor?" << endl;
      cin >> stage_decim_rate[stage];
      composite *= stage_decim_rate[stage];
    }
    if(composite == decim_factor) break;
      cout << "Error - product of specified factors \n"
           << "does not equal overall decimation rate" << endl;
  }

  cout << "original sampling rate (float)?" << endl;
  cin >> orig_samp_rate;
  cout << "edge frequency for final passband?" << endl;
  cin >> passband_edge_freq;
  stopband_edge_freq = orig_samp_rate/decim_factor/2.0;
  cout << "require odd number of taps? (0 = no, 1 = yes)" << endl;
  cin >> num_taps_must_be_odd;
  if(num_taps_must_be_odd !=0) num_taps_must_be_odd = 1;


  MultistageDecimCost( num_stages,
                       stage_decim_rate,
                       orig_samp_rate,
                       passband_edge_freq,
                       passband_ripple,
                       stopband_ripple,
                       num_taps_must_be_odd);
  delete[] stage_decim_rate;
  return 0;
 }  

