 //-----------------------------------------------------------------
 //
 //  file = prog_13a.cpp
 //
 //  Program for Frequency-Sampled FIR Filters
 //
 //-----------------------------------------------------------------
 #include <stdlib.h>
 #include <stdio.h> 
 #include <iostream.h> 
 #include <fstream.h>
 #include <math.h>
 #include "misdefs.h"
 #include "typedefs.h" 
 #include "goldsrch.h"
 #include "fs_util.h"
 #include "fs_spec.h"
 #include "optmiz2.h"
 #include "dirform1.h"
 #include "swept.h"
 #ifdef _DEBUG  
 ofstream DebugFile("fsamp.out", ios::out);
 #endif

 ofstream LogFile("search.log", ios::out);
 ofstream ResponseFile("fs_resp.txt", ios::out);
 ofstream CoeffFile("fs_coeff.out", ios::out);
 logical PauseEnabled;

 main()
 {
  int num_pts_contin_resp;
  double gold_sect_tol, tweak_factor;
  double y_base, z_base, z_work;
  double old_min, min_func_val;
  double slopes[4], origins[4], rect_comps[3];
  double x1;
  logical db_scale_enabled;
  logical normalize_enabled;
  logical swept_tone_enabled;
  logical rounding_enabled;
  logical post_quan_enabled, design_quan_enabled;
  int num_taps;
  long input_quan_factor;
  long coeff_quan_factor;
  long design_quan_factor;
  long post_quan_factor;
  double sampling_interval;
  FreqSampFilterSpec *filter_spec;
  FreqSampFilterDesign *filter_design;
  FreqSampFilterResponse *filter_response;
  DirectFormFir *filter_implem;
  SweptResponse *swept_response;
 
  PauseEnabled = FALSE;
  
  cout << "Frequency Sampling Method FIR Response" << endl; 
    
  y_base = 1.0;
  z_base = 1.0;
  z_work = z_base;
  old_min = 0.0;
       
  
  filter_spec = new FreqSampFilterSpec( cin, cout );

  //-----------------------------------------------------                                        
  if (filter_spec->IsOptimizeEnabled())
    {
     cout << "tolerance for golden section search?" << endl;
     cin >> gold_sect_tol;
     
     cout << "tweak factor for golden section search?" << endl;
     cin >> tweak_factor;
    }
  
  cout << "number of points for frequency response evaluations?" << endl;
  cin >> num_pts_contin_resp; 
  
  //-----------------------------------------------------------         
  cout << "Scale type?\n"
       << "  0 = linear\n"
       << "  1 = dB scale" << endl;
  cin >> db_scale_enabled;
  
  //----------------------------------------------------------
  cout << "Normalization type?\n"
       << "  0 = no normalization" << endl;
  if(db_scale_enabled)
    { cout << "  1 = set passband peak to 0.0 dB" << endl;}
  else
    { cout << "  1 = set passband peak to 1.0" << endl;}
  cin >> normalize_enabled;
                          
  //----------------------------------------------------------------------
  cout << "should coefficients be quantized\n"
       << "during design process?\n"
       << "  0 = NO,  1 = YES"  << endl;
  cin >> design_quan_enabled;
  design_quan_factor = 0;
  if(design_quan_enabled)
    {
     cout << "quantizing factor for coefficient design?"<< endl;
     cin >> design_quan_factor;
    }
  //----------------------------------------------------------------- 
  cout << "how should final freq response be evaluated?\n"
       << "  0 = use formulas\n"
       << "  1 = measure swept-tone response\n"
       << endl;
  cin >> swept_tone_enabled;
  
  //----------------------------------------------------------------------
  
  if(swept_tone_enabled)
    {
     cout << "sampling interval?" << endl;
     cin >> sampling_interval;
     cout << "quantizing factor for input signal?" << endl;
     cin >> input_quan_factor;
     cout << "quantizing factor for coefficients?" << endl;
     cin >> coeff_quan_factor;
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
       }
    }
  cout << "quantization method?\n"
	   << " 0 = truncation\n"
	   << " 1 = rounding" << endl;
  cin >> rounding_enabled;
     
  //----------------------------------------------------------------------  
  filter_design = new FreqSampFilterDesign( *filter_spec );
  
  filter_response = new FreqSampFilterResponse( filter_design, 
                                                num_pts_contin_resp,
                                                db_scale_enabled);
  //----------------------------------------------------------------------
  
  if (!(filter_spec->IsOptimizeEnabled()))
    {
     filter_design->ComputeCoefficients(filter_spec);
     filter_response->ComputeMagResp(filter_design, db_scale_enabled);
     if(normalize_enabled)
       {
        filter_response->NormalizeResponse(db_scale_enabled);  
       }
     filter_response->DumpMagResp( &ResponseFile );
     min_func_val = filter_response->GetStopbandPeak();
     cout << "peak ripple = " << min_func_val << endl;    
    }
  else
    {
     switch (filter_spec->GetNumTransSamps())
       {
        case 1:
          slopes[1] = 1.0;
          origins[1] = 0.0;
            
          x1 = GoldenSearch( gold_sect_tol,
                             filter_spec, 
                             filter_design,
                             filter_response,
                             design_quan_factor,
                             &min_func_val);
          DumpRectangCompon( origins, slopes, 1, x1);

          break;
        case 2:
          optimize2( filter_spec,
                     filter_design,
                     filter_response,
                     double(y_base),
                     double(gold_sect_tol),
                     double(tweak_factor),
                     rect_comps);
          cout << "Hb = " << rect_comps[0] << endl;
          cout << "Ha = " << rect_comps[1] << endl;
          pause(TRUE);
          break;
        case 3:
          break;           
       }  // end of switch on (filter_spec->GetNumTransSamps())

     if(swept_tone_enabled)
       {
        num_taps = filter_design->GetNumTaps();
        
        delete filter_response;
        filter_implem = new DirectFormFir( num_taps,
                                           filter_design->GetCoefficients(),
                                           TRUE,
                                           coeff_quan_factor,
                                           input_quan_factor );
        swept_response = new SweptResponse( filter_implem,
                                            sampling_interval,
                                            cin, cout );
        swept_response->DumpMagResp();
        delete swept_response;
       }
     else
       {
        if(post_quan_enabled)
          {
           filter_design->DumpCoefficients( &CoeffFile );
           filter_design->QuantizeCoefficients( post_quan_factor, 
			                                    rounding_enabled );
           filter_design->DumpCoefficients( &CoeffFile );
           
           filter_response->ComputeMagResp( filter_design, 
			                                db_scale_enabled);
           if(normalize_enabled)
             filter_response->NormalizeResponse(db_scale_enabled);
           filter_response->DumpMagResp( &ResponseFile );
          }
        else
          {
           filter_design->DumpCoefficients( &CoeffFile );
           if(normalize_enabled)
             filter_response->NormalizeResponse(db_scale_enabled);
           filter_response->DumpMagResp( &ResponseFile );
          } 
       }
    }  // end of else clause on if(!(filter_spec->IsOptimizeEnabled()))
  
       
  return 0;
 }  

