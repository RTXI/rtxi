 //-----------------------------------------------------------------
 //
 //  file = iir_fund.cpp
 //
 //  Program for IIR Filters
 //
 //-----------------------------------------------------------------
 #include <stdlib.h> 
 #include <iostream.h> 
 #include "misdefs.h"
 #include "typedefs.h" 
 #include "filtfunc.h"
 #include "buttfunc.h"
 #include "chebfunc.h"
 #include "elipfunc.h"
 #include "iir_dsgn.h"
 #include "iir_resp.h"
 #include "bilinear.h"
 #include "iirswept.h"
 #include "dir1_iir.h"
 #include "unq_iir.h"
 
 #ifdef _DEBUG
 ofstream DebugFile("iir_fund.out", ios::out);
 #endif

 ofstream LogFile("iir_fund.log", ios::out);
 ofstream ResponseFile("iir_resp.txt", ios::out);
 ofstream AnalogWaveFile("anlg_rcn.txt",ios::out);

 main()
 {
  FilterTransFunc *analog_filter;
  IirFilterDesign *filter_design;
  IirFilterResponse *filter_response;
  double sampling_interval, sampling_rate;
  double passband_ripple, stopband_ripple;
  double passband_edge, stopband_edge;
  int filter_order;
  int ripple_bw_norm;
  int type_of_filter;
  logical rounding_enabled;
  logical swept_tone_enabled;
  logical post_quan_enabled, quant_enabled;
  logical coef_scaling_enab;
  logical predistort_enabled;
  double coef_scale_factor;
  long input_quan_factor;
  long coeff_quan_factor;
  long post_quan_factor;
  FilterImplementation *filter_implem;
  SweptResponse *swept_response;

  cout << "sampling rate?" << endl;
  cin >> sampling_rate;
  sampling_interval = 1.0/sampling_rate;

  cout << "Desired type of filter:\n"
       << "  1 = Butterworth\n"
       << "  2 = Chebyshev\n"
       << "  3 = Elliptical\n"
       << endl;
  cin >> type_of_filter;
  
  cout << "Desired order of filter: " << endl;
  cin >> filter_order;
  
  if((type_of_filter == 2) || (type_of_filter == 3))
    {
     cout << "Desired passband ripple: " << endl;
     cin >> passband_ripple;
    }
  
  if(type_of_filter == 3)
    {
     cout << "Desired stopband ripple: " << endl;
     cin >> stopband_ripple;
    }
  
  cout << "Desired passband edge: (in Hz)" << endl;
  cin >> passband_edge;
  passband_edge *= TWO_PI;
  
  if( type_of_filter == 2 )
    {
     cout << "Desired type of normalization?\n"
          << "  0 = 3 dB bandwidth\n"
          << "  1 = ripple bandwidth" << endl;
     cin >> ripple_bw_norm;
    }
  
  if(type_of_filter == 3)
    {
     cout << "Desired stopband edge: (in Hz)" << endl;
     cin >> stopband_edge;
     stopband_edge *= TWO_PI;
    }
  

  switch (type_of_filter)
  {
   case 1:      //Butterworth
     analog_filter = new ButterworthTransFunc(filter_order);
     analog_filter->LowpassDenorm(passband_edge);
     break;
   case 2:     //Chebyshev
     analog_filter = new ChebyshevTransFunc( filter_order,
                                            passband_ripple,
                                            ripple_bw_norm);
     analog_filter->LowpassDenorm(passband_edge);
     break;
   case 3:   // Elliptical
     int upper_summation_limit = 5;
     analog_filter = new EllipticalTransFunc( 
                                   filter_order, 
                                   passband_ripple,
                                   stopband_ripple,
                                   passband_edge,
                                   stopband_edge,
                                   upper_summation_limit);
     #ifdef _DEBUG
     analog_filter->DumpBiquads(&DebugFile);
     #endif
     //analog_filter->LowpassDenorm(
     //                sqrt(passband_edge * stopband_edge));
     break;
  }
  cout << "predistort frequencies for bilinear transform?\n"
       << " 0 = NO,   1 = YES" << endl;
  cin >> predistort_enabled;

  if( predistort_enabled ) 
       analog_filter->FrequencyPrewarp( sampling_interval );

  filter_design = BilinearTransf( analog_filter,
                                  sampling_interval);  
  #ifdef _DEBUG
  filter_design->DumpCoefficients(&DebugFile);
  #endif

  //----------------------------------------------------------------- 
  cout << "how should final freq response be evaluated?\n"
       << "  0 = use formulas\n"
       << "  1 = measure swept-tone response\n"
       << endl;
  cin >> swept_tone_enabled;
  
  //--------------------------------------------------------
  if(swept_tone_enabled)
    {
     cout << "should signal and coefficients be quantized?\n"
          << "  0 = NO,  1 = YES" << endl;
     cin >> quant_enabled;
     if(quant_enabled)
     {
       cout << "quantizing factor for input signal?" << endl;
       cin >> input_quan_factor;
       cout << "quantizing factor for coefficients?" << endl;
       cin >> coeff_quan_factor;
     }
    }
  else
    {
     cout << "should coefficients be quantized\n"
          << "after design process is completed?\n"
          << "  0 = NO,  1 = YES"  << endl;
     cin >> post_quan_enabled;
     if(post_quan_enabled)
       {
        cout << "quantizing factor?"<< endl;
        cin >> post_quan_factor;
        cout << "type of quantization?\n"
             << " 0 = truncation\n"
             << " 1 = rounding" << endl;
        cin >> rounding_enabled;
        cout << "should coefficients be scaled prior to quantization?\n"
             << "  0 = NO,  1 = YES" << endl;
        cin >> coef_scaling_enab;
        if(coef_scaling_enab)
          {
           cout << "scaling factor?" << endl;
           cin >> coef_scale_factor;
          }
       }
    }
  if(swept_tone_enabled)
    {
      if(quant_enabled)
      {
        filter_implem = new DirectFormIir(
                         filter_design->GetNumNumerCoeffs(),
                         filter_design->GetNumDenomCoeffs(),
                         filter_design->GetNumerCoefficients(),
                         filter_design->GetDenomCoefficients(),
                         coeff_quan_factor,
                         input_quan_factor);
      }
      else
      {
        filter_implem = new UnquantDirectFormIir(
                         filter_design->GetNumNumerCoeffs(),
                         filter_design->GetNumDenomCoeffs(),
                         filter_design->GetNumerCoefficients(),
                         filter_design->GetDenomCoefficients());
      }
     swept_response = new SweptResponse( filter_implem,
                                         sampling_interval,
                                         cin, cout );
     swept_response->NormalizeResponse();
     swept_response->DumpMagResp();
     delete swept_response;
    }
  else
    {
     if(post_quan_enabled)
      {filter_design->QuantizeCoefficients( 
                                  post_quan_factor,
                                  rounding_enabled);
      }
     filter_response = 
                 new IirFilterResponse( filter_design, 
                                        cin,
                                        cout); 
     filter_response->ComputeResponse();
     filter_response->NormalizeResponse();
     filter_response->DumpMagResp();
    }
  return 0;
 }  

