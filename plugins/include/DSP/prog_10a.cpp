//
//  File = prog_10a.cpp
//
//
//  Program for Windows for Filtering and
//              Spectral Analysis
//

#include <fstream.h> 
#include <stdlib.h>
#include "misdefs.h"
#include "fir_resp.h"
#include "fir_dsgn.h"
#include "con_resp.h"
#include "con_rect.h"
#include "con_tngl.h"
#include "con_hamm.h"
#include "con_hann.h"
#include "rectnglr.h"
#include "trianglr.h"
#include "hamming.h"
#include "kaiser.h"
#include "hann.h"
#include "dolph.h"

#ifdef _DEBUG
  ofstream DebugFile("window.bug", ios::out);
#endif

ofstream ResponseFile("win_resp.txt", ios::out);
ofstream CoeffFile("wn_coeff.txt", ios::out);

main()
{
 int window_shape;
 int des_action;
 int num_taps;
 double *window_coeffs;
 logical post_quan_enabled;
 logical swept_tone_enabled;
 logical rounding_enabled;
 logical coef_scaling_enab;
 double coef_scale_factor;
 double sampling_interval;
 long input_quan_factor;
 long coeff_quan_factor;
 long post_quan_factor;
 FirFilterDesign *fir_design;
 FirFilterResponse *fir_response;
 ContinWindowResponse *cont_resp;
 GenericWindow *disc_window;
 
 for(;;)
 {
  //--------------------------------------------
  // select desired action
  cout << "Desired action?\n"
       << " 0 = quit\n"
       << _PLOT_CT_MAG_RESP << " = plot frequency response for cont-time window\n"
       << _GEN_DT_WIN_COEFFS << " = gen coeffs for discrete-time window\n"
       << _PLOT_DTFT_FOR_DT_WIN << " = plot DTFT for discrete-time window\n"
       << endl; 
  cin >> des_action;
  if( (des_action<1) || (des_action>5))
    { exit(0);}
       
  //--------------------------------------------
  //  Input desired window shape
  
  cout << "shape of window?\n  "
       << "0 = Quit\n  "
       << _RECTANGULAR << " = rectangular\n  "
       << _TRIANGULAR << " = triangular\n  "
       << _HAMMING << " = Hamming\n  "
       << _HANN << " = Hann (hanning, vonHann)\n  ";
  if( des_action > _PLOT_CT_MAG_RESP)
    {
    // continuous-time windows not supported for 
    // Dolph-Chebyshev and Kaiser
    cout << _DOLPH_CHEBY << " = Dolph-Chebyshev\n  "
         << _KAISER << " = Kaiser\n";
    }
  cout << endl;
  cin >> window_shape;
  
  if( window_shape < _RECTANGULAR)
    { exit(0);}
    
  //----------------------------------------------------
  // input desires relative to quantization
  // (only for actions involving discrete-time windows)
  
  if( des_action >= _GEN_DT_WIN_COEFFS )
    {
     //----------------------------------------------------------------- 
     cout << "how should final freq response be evaluated?\n"
          << "  0 = use formulas\n"
          << "  1 = measure swept-tone response\n"
          << endl;
     cin >> swept_tone_enabled;
  
     //--------------------------------------------------------
     if(swept_tone_enabled)
       {
        cout << "sampling_interval?"<< endl;
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
           cout << "type of quantization?\n"
                << " 0 = truncation\n"
                << " 1 = rounding" << endl;
           cin >> rounding_enabled;
           cout << "should coefficients be scaled prior to quantization?\n"
                << "  0 = NO,  1 = YES" << endl;
           cin >> coef_scaling_enab;
           if(coef_scaling_enab)
             {
              coef_scale_factor = (post_quan_factor-1.)/post_quan_factor;
             }
          }
       }
    }
  
  //------------------------------------------------------------  
  
  if( des_action == _PLOT_CT_MAG_RESP ) 
    {
     switch (window_shape)
       {
       case _RECTANGULAR:
         cont_resp = new ContRectangularMagResp(cin, cout);
         break;
       case _TRIANGULAR:
         cont_resp = new ContTriangularMagResp(cin, cout);
         break;
       case _HAMMING:
         cont_resp = new ContHammingMagResp(cin, cout);
         break;
       case _HANN:
         cont_resp = new ContHannMagResp(cin, cout);
         break;
       case _DOLPH_CHEBY:
       case _KAISER:
       default:
         break;
       }
     ResponseFile.close();
     exit(2);
    }

  //------------------------------------------------------
  
  switch (window_shape)
    {
    case _RECTANGULAR:     // rectangular
      #ifdef _DEBUG
      DebugFile << "Rectangular window" << endl;
      #endif
      disc_window = new RectangularWindow( cin, cout );
      break;
    case _TRIANGULAR:     // triangular
      #ifdef _DEBUG
      DebugFile << "Triangular window" << endl;
      #endif
      disc_window = new TriangularWindow( cin, cout );
      cout << "Just finished triangular window" << endl;
      break;
    case _HAMMING:     // Hamming
      #ifdef _DEBUG
      DebugFile << "Hamming window" << endl;
      #endif
      disc_window = new HammingWindow( cin, cout );
      break;
    case _HANN:     // Hann
      #ifdef _DEBUG
      DebugFile << "Hann window" << endl;
      #endif
      disc_window = new HannWindow( cin, cout );
      break;
    case _DOLPH_CHEBY:     // Dolph-Chebyshev 
      #ifdef _DEBUG
      DebugFile << "Dolph-Chebyshev window" << endl;
      #endif
      disc_window = new DolphChebyWindow( cin, cout);
      break;
    case _KAISER:
      #ifdef _DEBUG
      DebugFile << "Kaiser window" << endl;
      #endif
      disc_window = new KaiserWindow( cin, cout);
      break;
    } // end of switch on window_shape
    
  switch (des_action)
    {
    case _GEN_DT_WIN_COEFFS:
      disc_window->DumpHalfLagWindow( &CoeffFile );
      break; 
      
    case _PLOT_DTFT_FOR_DT_WIN:
      window_coeffs = disc_window->GetDataWindow();
      num_taps = disc_window->GetNumTaps();
      fir_design = new FirFilterDesign(num_taps, window_coeffs);
      if(post_quan_enabled) 
        {
         fir_design->DumpCoefficients(&CoeffFile);
         
         if(coef_scaling_enab)
           {
            fir_design->ScaleCoefficients( coef_scale_factor );
           }
           
         fir_design->QuantizeCoefficients( post_quan_factor,
                                           rounding_enabled);
        }
      fir_design->DumpCoefficients(&CoeffFile);
      fir_response = new FirFilterResponse( fir_design, cin, cout);
      fir_response->ComputeMagResp();
      fir_response->DumpMagResp();
      break; 
            
    }  // end of switch on des_action 
   
 // ResponseFile.close();
    
 }
 return 0;
}