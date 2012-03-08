//
//  file = prog_20b.cpp
//
//
#include <stdlib.h> 
#include <iostream.h> 
#include <fstream.h>
#include <math.h>
#include "misdefs.h"
#include "filtmath.h"
#include "abstmath.h"
#include "msmrcost.h"
    

#ifdef _DEBUG
 ofstream DebugFile("multstag.bug", ios::out);
#endif

 main()
 {
  double passband_ripple, stopband_ripple;
  double highest_samp_rate;
  int overall_rate_change, num_stages, stage;
  double passband_edge_freq, stopband_edge_freq;
  int composite;
  int num_taps_must_be_odd;
  PrimeFactorSet *prime_factors;

  cout << "desired overall rate change factor?" << endl;
  cin >> overall_rate_change;

  cout << "highest sampling rate (float)" << endl;
  cout << "  (input rate for decimator)" << endl;
  cout << "  (output rate for interpolator)" << endl;
  cin >> highest_samp_rate;
  cout << "edge frequency for final passband?" << endl;
  cin >> passband_edge_freq;

  cout << "total passband ripple?" << endl;
  cin >> passband_ripple;
  
  cout << "total stopband ripple?" << endl;
  cin >> stopband_ripple;

  prime_factors = PrimeFactorization( overall_rate_change );

  cout << "number of stages?" << endl;
  cin >> num_stages;

  int* stage_factors = new int[num_stages];

  for(;;)
  {
    cout << "\nfor decimator, stage 0 is first stage" << endl;
    cout << "for interpolator, stage 0 is last stage" << endl;
    composite = 1;
    for(stage=0; stage<num_stages; stage++)
    {
      cout << "stage "<< stage << " factor?" << endl;
      cin >> stage_factors[stage];
      composite *= stage_factors[stage];
    }
    if(composite == overall_rate_change) break;
      cout << "Error - product of specified stage factors \n"
           << "does not equal overall rate change factor" << endl;
  }

  stopband_edge_freq = highest_samp_rate/overall_rate_change/2.0;
  cout << "require odd number of taps? (0 = no, 1 = yes)" << endl;
  cin >> num_taps_must_be_odd;
  if(num_taps_must_be_odd !=0) num_taps_must_be_odd = 1;


  MultistageMultirateCost( num_stages,
                           stage_factors,
                           highest_samp_rate,
                           passband_edge_freq,
                           passband_ripple,
                           stopband_ripple,
                           num_taps_must_be_odd);
  delete[] stage_factors;
  return 0;
 }  

