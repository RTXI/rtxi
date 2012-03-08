//
//  file = prog_05a.cpp
//

 #include <stdlib.h> 
 #include <iostream.h> 
 #include <fstream.h>
 #include <math.h> 
 #include "misdefs.h"
 #include "polefilt.h"
 #include "pzfilt.h"
 #include "anlgfilt.h"
 #include "cmpxpoly.h"
 #include "poly.h"
 #include "filtfunc.h"
 #include "buttfunc.h"
 #include "bessfunc.h"
 #include "chebfunc.h"
 #include "elipfunc.h"
 #include "impresp.h"
 #include "stepresp.h"
  
 #ifdef _DEBUG
 ofstream DebugFile("analog.out", ios::out);
 #endif

 //======================================================
 //  main program for analog filter work
 //  Chapter 5
 //  freq response, impulse response, step response
 //  everything but simulation -- 
 //  main for simulation is in file prog_05b.cpp

 main()
 {
  double delta_t;
  float total_time;
  double passband_ripple, stopband_ripple;
  double passband_edge, stopband_edge;
  int order; 
  int num_resp_pts;
  int type_of_filter; 
  int type_of_test_signal;
  int ripple_bw_norm, norm_for_delay;
  int freqs_in_hz;
  AnalogFilter* filter;
  ImpulseResponse* impulse_response;
  StepResponse* step_response;
  
  CmplxPolynomial cmplx_denom_poly;
  Polynomial denom_poly;
  CmplxPolynomial cmplx_numer_poly;
  Polynomial numer_poly;
  FilterTransFunc* filter_function;
  
  cout << "Desired type of filter:\n"
       << "  1 = Butterworth\n"
       << "  2 = Chebyshev\n"
       << "  3 = reserved\n"
       << "  4 = Elliptical\n"
       << "  5 = Bessel\n"
       << endl;
  cin >> type_of_filter;
  
  cout << "Desired order of filter: " << endl;
  cin >> order;

  cout << "Units for specified frequencies?\n"
       << "   0 = radians per second\n"
       << "   1 = Hz" << endl;
  cin >> freqs_in_hz;
  
  if((type_of_filter == 2) || (type_of_filter == 4))
    {
     cout << "Desired passband ripple: " << endl;
     cin >> passband_ripple;
    }
  
  if((type_of_filter == 3) || (type_of_filter == 4))
    {
     cout << "Desired stopband ripple: " << endl;
     cin >> stopband_ripple;
    }
  
  cout << "Desired passband edge: " << endl;
  cin >> passband_edge;
  if(freqs_in_hz) passband_edge *= TWO_PI;
  
  if( type_of_filter == 2 )
    {
     cout << "Desired type of normalization?\n"
          << "  0 = 3 dB bandwidth\n"
          << "  1 = ripple bandwidth" << endl;
     cin >> ripple_bw_norm;
    }
  
  if(type_of_filter == 4)
    {
     cout << "Desired stopband edge: " << endl;
     cin >> stopband_edge;
     if(freqs_in_hz) stopband_edge *= TWO_PI;
    }

  if(type_of_filter == 5)
    {
    cout << "Desired type of normalization?\n"
         << "  0 = 3 dB attenuation at PB edge\n"
         << "  1 = unit delay at zero frequency"
         << endl;
    cin >> norm_for_delay;
    }
  
  cout << "Desired type of response:\n"
       << "   0 = frequency response\n"
       << "   1 = impulse response\n"
       << "   2 = step response" << endl;
  cin >> type_of_test_signal;
  
  if(type_of_test_signal >= 1)
    {
    try_again:
    cout << "Desired time increment: " << endl;
    cin >> delta_t;
  
    if(type_of_filter ==4)
      {
      //if( (1./delta_t) < (2.0*stopband_edge) )
      if( (PI/delta_t) < stopband_edge )
        {
        cout << "for this filter, time increment must be < "
             << PI/stopband_edge << endl;
        goto try_again;
        }
      } 
    else
      {
      //if( (1./delta_t) < (5.0*passband_edge) )
      if( (PI/delta_t) < (5.0*passband_edge) )
        {
        cout << "for this filter, time increment must be < "
             << (0.2 * PI) /passband_edge << endl;
        goto try_again;
        }
      }  
    
    cout << "number of points in plot?" << endl;
    cin >> num_resp_pts;
    }  // end of if(type_of_test_signal >= 1)
  int upper_summation_limit = 5;
  switch (type_of_filter)
    {
    case 1:          // Butterworth
      filter_function = new ButterworthTransFunc(order);
      filter_function->LowpassDenorm(passband_edge);
      break;
    case 2:
      filter_function = new ChebyshevTransFunc( 
                                   order, 
                                   (float)passband_ripple,
                                   ripple_bw_norm);
      filter_function->LowpassDenorm(passband_edge);
      break;
    case 3:
      cout << "Chebyshev type II not supported" << endl;
      return 0;
    case 4:
      filter_function = new EllipticalTransFunc( 
                                    order, 
                                    passband_ripple,
                                    stopband_ripple,
                                    passband_edge,
                                    stopband_edge,
                                    upper_summation_limit);
      #ifdef _DEBUG
      filter_function->DumpBiquads(&DebugFile);
      #endif
      filter_function->LowpassDenorm(
                       sqrt(passband_edge * stopband_edge));
      break;
    case 5:
      filter_function = new BesselTransFunc(
                                order,
                                passband_edge,
                                norm_for_delay);
      filter_function->LowpassDenorm(passband_edge);
      break;
    }
  if( type_of_test_signal == 0) // frequency response
    {
    filter_function->FilterFrequencyResponse();
    return 0;
    }
  
  if(type_of_filter == 4)
    {
     filter = new AnalogPoleZeroFilter( filter_function->GetNumerPoly(),
                                        filter_function->GetDenomPoly(),
                                        filter_function->GetHSubZero(),
                                        delta_t);
    }
  else
    {
     filter = new AnalogAllPoleFilt( filter_function->GetDenomPoly(),
                                     filter_function->GetHSubZero(),
                                     delta_t);
    }

 
  long int k_stop = (long int)(total_time/delta_t);
  cout << "k_stop = " << k_stop << endl;  

  switch (type_of_test_signal)
    {
    case 0:   // frequency response
      break;
    case 1:   // impulse response
      impulse_response =
                new ImpulseResponse( filter_function,
                                     num_resp_pts,
                                     delta_t);
      impulse_response->GenerateResponse();
      break;
    case 2:   //step response
      step_response =
                new StepResponse( filter_function,
                                  num_resp_pts,
                                  delta_t);
      step_response->GenerateResponse();
      break;               
    }
     
  return 0;
 }  

