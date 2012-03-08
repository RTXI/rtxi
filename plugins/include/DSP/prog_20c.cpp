 #include <stdlib.h> 
 #include <iostream.h> 
 #include <fstream.h>
 #include <math.h>
 #include "filtmath.h"
 #include "misdefs.h"
 #include "dirform1.h"
 #include "anlg_rcn.h" 
 #include "fir_resp.h"
 #include "remezalg.h"
 #include "croscorr.h"
#include "mr_lpf.h"
   

#ifdef _DEBUG
 ofstream DebugFile("multirat.bug", ios::out);
#endif

 main()
 {
  RemezAlgorithm *remez;
  FirFilterDesign *fir_filter;
  FilterImplementation *filter_implem;
  FilterImplementation *multirate_implem;
  int filter_length;
  double passband_edge_freq, nrmlzd_passband_edge;
  double stopband_edge_freq, nrmlzd_stopband_edge;
  double passband_ripple, stopband_ripple;
  double nrmlzd_trans_width;
  double samp_freq;
  double d_sub_inf;
  long input_quan_fact;
  long coef_quan_fact;
  long quan_in_val, quan_out_val;
  int estim_filt_len;
  int query_resp;
  int recon_beg = 400;
  int recon_end = 500;

  int analog_interp_rate = 20;
  int num_signif_sidelobes = 100;
  int num_anlg_input_samps;
  int num_anlg_decim_out_samps;

  double input_val, decim_out_val;
  double interp_out_val;
  int samp_indx, decim_rate, interp_rate;
  int decim_samp_indx, rho;
  int freq_indx, freq_indx_beg, freq_indx_end, swept_resp_len;
  logical quan_enab;
  
  cout << "sampling frequency?" << endl;
  cin >> samp_freq;

  cout << "passband edge frequency?" << endl;
  cin >> passband_edge_freq;
  nrmlzd_passband_edge = passband_edge_freq/samp_freq;
  
  cout << "stopband edge frequency?" << endl;
  cin >> stopband_edge_freq;
  nrmlzd_stopband_edge = stopband_edge_freq/samp_freq;
  nrmlzd_trans_width = nrmlzd_stopband_edge 
                      - nrmlzd_passband_edge;

  cout << "passband ripple?" << endl;
  cin >> passband_ripple;

  cout << "stopband ripple?" << endl;
  cin >> stopband_ripple;

  d_sub_inf = DSubInf(passband_ripple, stopband_ripple);
  cout << "D_inf is " << d_sub_inf << endl;
  
  estim_filt_len = int(ceil(d_sub_inf/nrmlzd_trans_width));

  cout << "filter length must be " << estim_filt_len
       << " or larger" << endl;

  read_filter_length:
  cout << "filter length ?\n (must be odd)" << endl;
  cin >> filter_length;

  if((filter_length%2) == 0) goto read_filter_length;

  cout << &cout << endl;
    
  remez = new RemezAlgorithm( cin, cout,
                              filter_length,
                              nrmlzd_passband_edge,
                              nrmlzd_stopband_edge,
                              passband_ripple/stopband_ripple,
                              &fir_filter);

  fir_filter->DumpCoefficients(&DebugFile);
  fir_filter->NormalizeFilter();
  fir_filter->DumpCoefficients(&DebugFile);
  FirFilterResponse* fir_resp = new FirFilterResponse( fir_filter,
                                                       cin, cout );

  fir_resp->ComputeMagResp();
  fir_resp->DumpMagResp();

  cout << "filter is designed, enter 0 to stop, 1 to continue" <<endl;
  cin >> query_resp;

  if(query_resp == 0)
  {
    delete remez;
    delete fir_resp;
    return 0;
  }
  

  int num_taps = fir_filter->GetNumTaps();
  double* coeff = new double[num_taps];
  fir_filter->CopyCoefficients(coeff);

  cout << "Quantize samples and coefficients?\n"
       << "( 0 = NO, 1 = YES )" << endl;
  cin >> quan_enab;

  if(quan_enab){    
  cout << "input quantizing factor?\n"
       << " ( 256 for 8 bits, 1024 for 10 bits, etc. )"
       << endl;
  cin >> input_quan_fact;

  cout << "coefficient quantizing factor?\n"
       << " ( 256 for 8 bits, 1024 for 10 bits, etc. )"
       << endl;
  cin >> coef_quan_fact;
  }
  else {
    input_quan_fact = 0;
    coef_quan_fact = 0;
  }

  filter_implem = new DirectFormFir( num_taps,
                                     coeff,
                                     quan_enab,
                                     coef_quan_fact,
                                     input_quan_fact);

  decim_rate = 5;
  multirate_implem = new MultirateLowpass( fir_filter,
                                           fir_filter,
                                           decim_rate,
                                           quan_enab,
                                           input_quan_fact,
                                           coef_quan_fact);

  //---------------------------------------------------------
  cout << "how many points in complete swept response?" 
       << endl;
  cin >> swept_resp_len;
  cout << "index of first sweep frequency?" << endl;
  cin >> freq_indx_beg;
  cout << "index of final sweep frequency?" << endl;
  cin >> freq_indx_end;

  ofstream swept_resp_file("mr_swept.txt",ios::out);

  interp_rate = 5;

  num_anlg_decim_out_samps = analog_interp_rate
                           * (1+recon_end-recon_beg);
  num_anlg_input_samps = interp_rate * num_anlg_decim_out_samps;

  double *swept_mag_resp;
  swept_mag_resp = new double[freq_indx_end+1];
  //-----------------------------------------------------------
  //  start main loop thru sweep frequencies
  //
  for( freq_indx=freq_indx_beg; 
       freq_indx<=freq_indx_end; 
       freq_indx++) {
    AnalogReconst* input_reconst;
    input_reconst = new AnalogReconst( 1.0/samp_freq,
                                       analog_interp_rate,
                                       num_signif_sidelobes,
                                       num_anlg_input_samps);

    AnalogReconst* interp_out_reconst;
    interp_out_reconst = new AnalogReconst( 1.0/samp_freq,
                                            analog_interp_rate,
                                            num_signif_sidelobes,
                                            num_anlg_input_samps);

    samp_indx = -1;
    for(decim_samp_indx=0; decim_samp_indx<500; decim_samp_indx++){
      decim_out_val = 0.0;
      for(rho=0; rho<decim_rate; rho++) {
        samp_indx++;
        //
        // Generate a new input sample for each polyphase filter
        input_val = cos((PI*double(freq_indx*samp_indx))/
                                  double(swept_resp_len));
        if(quan_enab) 
              quan_in_val = long(input_quan_fact * input_val);
        if( (decim_samp_indx >= recon_beg) &&
            (decim_samp_indx < recon_end) )
          {
          input_reconst->AddSample(input_val);
          }
        if(quan_enab)
          {
          quan_out_val = 
                 multirate_implem->ProcessSample(quan_in_val);
          interp_out_val = 
                 double(quan_out_val)/double(input_quan_fact);
          }
        else
          {
          interp_out_val = 
                 //filter_implem->ProcessSample(input_val);
                 multirate_implem->ProcessSample(input_val);
          }
        if( (decim_samp_indx >= recon_beg) &&
            (decim_samp_indx < recon_end) ) {
          interp_out_reconst->AddSample(interp_out_val);
          }
      } // end of loop over rho

      // at this point, one new output sample is ready
      if((decim_samp_indx%20)==0) {
        cout << freq_indx << " : " << decim_samp_indx 
             << "\r" << flush;
      }
    }  // end of loop over decim_samp_indx


    if(freq_indx == freq_indx_end)
    {
      ofstream sim_file_3("mr_sim3.txt",ios::out);
      input_reconst->DumpResult(&sim_file_3);
      sim_file_3.close();

      ofstream sim_file_5("mr_sim5.txt",ios::out);
      interp_out_reconst->DumpResult(&sim_file_5);
      sim_file_5.close();
    }

    //------------------------------------------------
    //  Correlate analog reconstructions of input and
    //  ultimate output to determine amplitude and
    //  phase change caused by the multirate structure.
    //
    double* input_segment = new double[num_anlg_input_samps];
    double* output_segment = new double[num_anlg_input_samps];
    double phase_delta, gain;
    int indx_of_peak;

    input_reconst->CopyResult(input_segment);
    interp_out_reconst->CopyResult(output_segment);

    CrossCorrelation( num_anlg_input_samps,
                      input_segment, 
                      output_segment,
                      &gain,
                      &phase_delta,
                      &indx_of_peak);
    swept_mag_resp[freq_indx] = 20.0*log10(gain);
    delete[] output_segment;
    delete[] input_segment;
    delete input_reconst;
    delete interp_out_reconst;
  } // end of loop over freq_indx

  double peak;
  peak = -9999.0;
  for( freq_indx=freq_indx_beg; 
       freq_indx<=freq_indx_end; 
       freq_indx++) {
    if(swept_mag_resp[freq_indx]>peak) 
                    peak = swept_mag_resp[freq_indx];
    }
  for( freq_indx=freq_indx_beg; 
       freq_indx<=freq_indx_end; 
       freq_indx++) {
    swept_resp_file << freq_indx << ", " 
                    << (swept_mag_resp[freq_indx]-peak)
                    << endl;
    }
  swept_resp_file.close();
  //low_rate_resp_file.close();
  delete remez;
  
  return 0;
 }  
