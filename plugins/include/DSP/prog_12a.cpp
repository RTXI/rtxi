//
//  File = prog_12a.cpp
//
//
//  Program for Window FIR Filters
//

#include <fstream.h> 
#include <stdlib.h>
#include "misdefs.h"
#include "firideal.h"
#include "fir_resp.h"
#include "lin_resp.h"
#include "fir_dsgn.h"
#include "dirform1.h"
#include "swept.h"
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
 int num_taps;
 logical post_quan_enabled;
 logical swept_tone_enabled;
 logical rounding_enabled;
 logical coef_scaling_enab;
 double coef_scale_factor;
 double sampling_interval;
 long input_quan_factor;
 long coeff_quan_factor;
 long post_quan_factor;
 FirFilterDesign *filter_design;
 FirFilterResponse *filter_response;
 GenericWindow *disc_window;
 DirectFormFir *filter_implem;
 SweptResponse *swept_response;
 
 for(;;)
 {
  //--------------------------------------------
  //  Input desired window shape
  
  cout << "shape of window?\n  "
       << "0 = Quit\n  "
       << _RECTANGULAR << " = rectangular\n  "
       << _TRIANGULAR << " = triangular\n  "
       << _HAMMING << " = Hamming\n  "
       << _HANN << " = Hann (hanning, vonHann)\n  "
       << _DOLPH_CHEBY << " = Dolph-Chebyshev\n  "
       << _KAISER << " = Kaiser\n"
       << endl;
  cin >> window_shape;
  
  if( window_shape < _RECTANGULAR)
    { exit(0);}
    
  //----------------------------------------------------
  // input desires relative to quantization
  // (only for actions involving discrete-time windows)
  
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
          //cout << "scaling factor?" << endl;
          //cin >> coef_scale_factor;
          coef_scale_factor = (post_quan_factor-1.)/post_quan_factor;
         }
      }
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
    
num_taps = disc_window->GetNumTaps();
cout << "num_taps from window = " << num_taps << endl;
filter_design = new FirIdealFilter(num_taps, cin, cout);
filter_design->ApplyWindow(disc_window);

if(swept_tone_enabled)
  {
   filter_implem = new DirectFormFir( 
                                 num_taps,
                                 filter_design->GetCoefficients(),
                                 0,
                                 coeff_quan_factor,
                                 input_quan_factor );

   swept_response = new SweptResponse( filter_implem,
                                       sampling_interval,
                                       cin, cout );
   //swept_response->NormalizeResponse();
   swept_response->DumpMagResp();
   delete swept_response;
  }
else
  {
   if(post_quan_enabled)
     {
      filter_design->DumpCoefficients(&CoeffFile);
      filter_design->QuantizeCoefficients(post_quan_factor, rounding_enabled);
     }
   filter_design->DumpCoefficients( &CoeffFile);
   filter_response = new LinearPhaseFirResponse( 
                               (LinearPhaseFirDesign*)filter_design, 
                               cin, cout);
   filter_response->ComputeMagResp();
   filter_response->DumpMagResp();
  } // end of else clause on if(swept_tone_enabled)

 } // end of for(;;)
 return 0;
}