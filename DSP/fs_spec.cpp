//
//  File = fs_spec.cpp
//

#include <iostream> 
#include <fstream>
#include <math.h>
#include "fs_spec.h"

  
FreqSampFilterSpec::FreqSampFilterSpec( int band_config,
                                        int fir_type,
                                        int num_taps,
                                        double *des_mag_resp)
{ 
 int fold_indx, n;
 
 Band_Config = band_config;
 Fir_Type = fir_type;
 Num_Taps = num_taps;
 N1 = 0;
 N2 = 0;
 N3 = 0;
 N4 = 0;
 Des_Mag_Resp = new double[num_taps];
 
 if(num_taps%2)
   {fold_indx = (num_taps+1)/2;}
 else
   {fold_indx = num_taps/2;}
      
 for(n=0; n<fold_indx; n++)
   {
    Des_Mag_Resp[n] = des_mag_resp[n];
   }
 return;
}

 
FreqSampFilterSpec::FreqSampFilterSpec( istream& uin,
                                        ostream& uout)
{ 
 int n; 
 double resp_val;
 double lambda_predic;
 #define _UNDEF_RESP_ 0
 #define _LOWPASS_RESP_ 1
 #define _BANDPASS_RESP_ 2
 #define _HIGHPASS_RESP_ 3
 #define _BANDSTOP_RESP_ 4
 
 uout << "type of response ?\n"
      << "  0 = quit\n"
      << "  1 = lowpass\n"
      << "  2 = bandpass\n"
      << "  3 = highpass\n"
      << "  4 = bandstop" 
      << std::endl;
 uin >> Band_Config;
 if((Band_Config < 1)||(Band_Config > 4)) exit(0);

 uout << "type of constant group delay FIR ?\n"
      << "  1 = even symmetry and odd length\n"
      << "  2 = even symmetry and even length\n"
      << "  3 = odd symmetry and odd length\n"
      << "  4 = odd symmetry and even length\n"
      << std::endl;
 uin >> Fir_Type;
 if( (Fir_Type < 1)||(Fir_Type > 4)) exit(0);
 
 //--------------------------------------------
 uout << "number of taps?" << std::endl;
 uin >> Num_Taps;
 
 if(Num_Taps%2)
   {Fold_Indx = (Num_Taps-1)/2;}
 else
   {Fold_Indx = Num_Taps/2;}
           
 Des_Mag_Resp = new double[Num_Taps];
 
 uout << "type of specified frequencies?\n"
      << "  0 = discrete frequencies specified via indices\n"
      << "  1 = normalized for folding frequency of PI\n"
      << "  2 = actual frequencies\n"
      << std::endl;
 uin >> Freq_Type;
                             
 switch (Freq_Type)
   {
   case 0:       // discrete freqs spec'ed via index
     switch (Band_Config)
       {
       //- - - - - - - - - - - - - - - - - - - - - - - - - - -
       case _LOWPASS_RESP_:
         uout << "index of last sample in passband?" << std::endl;
         uin >> N1; 
       
         uout << "index of first sample in stopband?" << std::endl;
         uin >> N2;
       
         N3 = 0;
         N4 = 0;

         Num_Trans_Samps = N2 - N1 - 1;
         break;
      
       //- - - - - - - - - - - - - - - - - - - - - - - - - - 
       case _BANDPASS_RESP_:
         break;
       } // end of switch on Band_Config for Freq_Type 0
     break; //end of case 0 for switch on Freq_Type
     
   //--------------------------------------------------  
   case 1:   // normalized for folding frequency of PI
     Freqs_Specif_In_Hz = FALSE;
     Samp_Freq = TWO_PI;
     uout << "type of cutoff frequencies?\n"
          << "  0 = half power (i.e. 3 dB freq)\n"
          << "  1 = half amplitude  (i.e. 6 dB freq)\n"
          << std::endl;
     uin >> Cutoff_Is_Half_Amp;
     
     switch (Band_Config)
       {
       case _LOWPASS_RESP_:
         uout << "cutoff frequency is (N/D) * PI rad/sec\n"
              << "  enter numerator N\n" 
              << std::endl;
         uin >> Lambda_Numer;
         uout << "  enter denominator D\n"
              << std::endl;
         uin >> Lambda_Denom;
         
         //uout << "cutoff frequency as fraction of PI" << std::endl;
        //uin >> Lambda;
         if(Num_Taps%2)  // odd number of taps
           {
            if(Cutoff_Is_Half_Amp)
              { 
               N1 = (int)floor(Num_Taps*Lambda_Numer/Lambda_Denom/2.0 - 0.5);
               //N1 = (int)floor(Num_Taps*Lambda/2.0 - 0.5);
               lambda_predic = 2.0*(N1 + 0.5)/Num_Taps;
              }
            else
              { 
               N1 = (int)floor(Num_Taps*Lambda_Numer/Lambda_Denom/2.0 - 0.293);
               //N1 = (int)floor(Num_Taps*Lambda/2.0 - 0.293);
               lambda_predic = 2.0*(N1 + 0.293)/Num_Taps;
              }
           }
         else  // even number of taps
           {
            if(Cutoff_Is_Half_Amp)
              {
               N1 = (int)floor(Num_Taps*Lambda_Numer/Lambda_Denom/2.0);
               lambda_predic = 2.0*N1/Num_Taps;
              }
            else
              {
               N1 = (int)floor(Num_Taps*Lambda_Numer/Lambda_Denom/2.0 + 0.207);
               lambda_predic = 2.0*(N1 - 0.207)/Num_Taps;
              }
           }
         Lambda = PI * Lambda_Numer/Lambda_Denom;
         //Lambda = PI * Lambda;
         uout << "last sample in passband is sample ["
              << N1 << "]" << std::endl;
         uout << "with no transition samples,\n"
              << " predicted cutoff is " 
              << lambda_predic << " times PI\n"
              << " or " << (lambda_predic*PI)
              << std::endl;
        /* uout << "cutoff error is " 
              << (lambda_predic - Lambda_Numer/Lambda_Denom)
              << " times PI\n or "
              << (PI*(lambda_predic - Lambda_Numer/Lambda_Denom))
              << std::endl; */
         uout << "desired number of transition samples?\n"
              << " (0, 1, or 2)" << std::endl;
         uin >> Num_Trans_Samps;
         N2 = N1 + 1 + Num_Trans_Samps;
         N3 = 0;
         N4 = 0;
         break;
       } // end of switch on Band_Config for Freq_Type 1
       
     break; // end of case 1 for switch on Freq_Type
   
   //--------------------------------------------------  
   case 2:   // actual frequencies
    uout << "Frequency units?\n"
         << "  0 = Hz\n"
         << "  1 = radians/sec\n"
         << std::endl;
    uin >> Freqs_Specif_In_Hz;

    uout << "Sampling frequency?\n" << std::endl;
    uin >> Samp_Freq;
    if(Freqs_Specif_In_Hz) Samp_Freq *= TWO_PI;
    
     uout << "type of cutoff frequencies?\n"
          << "  0 = half power (i.e. 3 dB freq)\n"
          << "  1 = half amplitude  (i.e. 6 dB freq)\n"
          << std::endl;
     uin >> Cutoff_Is_Half_Amp;
    
     switch (Band_Config)
       {
       case _LOWPASS_RESP_:
         uout << "cutoff frequency?" << std::endl;
         uin >> Lambda;
         if(Freqs_Specif_In_Hz) Lambda *= TWO_PI;
       } // end of switch on Band_Config for Freq_Type 2
       
     break;  // end of case 2 for switch on Freq_Type 
     
   }  // end of switch on Freq_Type
  
  //-----------------------------------------------------
  //  For desired magnitude response, initialize
  //    passband(s) to 1.0  and stopband(s) to 0.0
  
  switch (Band_Config)
    {
    case _LOWPASS_RESP_:
      for(n=0; n<=N1; n++) Des_Mag_Resp[n] = 1.0;
      for(n=N2; n<=Fold_Indx; n++) Des_Mag_Resp[n] = 0.0;
      break;
    case _BANDPASS_RESP_:
      for(n=0; n<=N1; n++) Des_Mag_Resp[n] = 0.0;
      for(n=N2; n<=N3; n++) Des_Mag_Resp[n] = 1.0;
      for(n=N4; n<=Fold_Indx; n++) Des_Mag_Resp[n] = 0.0;
      break;
    case _HIGHPASS_RESP_:
      for(n=0; n<=N1; n++) Des_Mag_Resp[n] = 0.0;
      for(n=N2; n<=Fold_Indx; n++) Des_Mag_Resp[n] = 1.0;
      break;
    case _BANDSTOP_RESP_:
      for(n=0; n<=N1; n++) Des_Mag_Resp[n] = 1.0;
      for(n=N2; n<=N3; n++) Des_Mag_Resp[n] = 0.0;
      for(n=N4; n<=Fold_Indx; n++) Des_Mag_Resp[n] = 1.0;
      break;
    }  
   
       
  //------------------------------------------------- 
  if(Num_Trans_Samps == 0)
    {
     Optimize_Enabled = 0;
    }
  else
    {
     uout << "transition sample placement:" << std::endl;
     uout << "  0 = manual, 1 = optimal" << std::endl;
     uin >> Optimize_Enabled;
  
     if(!Optimize_Enabled)
       {
        for( n=N1+1; n<N2; n++)
          {
           uout << "magnitude response sample [" << n
                << "] ?" << std::endl;
           uin >> resp_val;
           Des_Mag_Resp[n] = resp_val;
          }
       }  // end of if(!Optimize_Enabled)
    } // end of else clause on if(Num_Trans_Samps == 0)
 return;  
}

FreqSampFilterSpec::FreqSampFilterSpec( const FreqSampFilterSpec &filter_spec)
{
 Band_Config = filter_spec.Band_Config;
 Num_Taps = filter_spec.Num_Taps;
 Fold_Indx = filter_spec.Fold_Indx;
 Fir_Type = filter_spec.Fir_Type;
 Optimize_Enabled = filter_spec.Optimize_Enabled;
 Num_Trans_Samps = filter_spec.Num_Trans_Samps;
 N1 = filter_spec.N1;
 N2 = filter_spec.N2;
 N3 = filter_spec.N3;
 N4 = filter_spec.N4;
 
 Des_Mag_Resp = new double[Num_Taps];
 for(int n=0; n<Num_Taps; n++)
   {
    Des_Mag_Resp[n] = filter_spec.Des_Mag_Resp[n];
   }

 return;
}


void FreqSampFilterSpec::SetTrans( double trans_val)
{
 switch (Band_Config)
   {
    case 1:    // lowpass
      Des_Mag_Resp[N2-1] = trans_val;
      break;
      
    case 2:    // bandpass
      Des_Mag_Resp[N1+1] = trans_val;
      Des_Mag_Resp[N4-1] = trans_val;
      
    case 3:    // highpass
      Des_Mag_Resp[N1+1] = trans_val;
      
    case 4:    // bandstop
      Des_Mag_Resp[N2-1] = trans_val;
      Des_Mag_Resp[N3+1] = trans_val;
      break;
   }
 
 return;
}


//=============================================================
void FreqSampFilterSpec::SetTrans( double *origins, 
                                   double *slopes, 
                                   double rho)
{
 int num_trans_samps, n;

 num_trans_samps = N2 - N1 - 1;
 
 switch (Band_Config)
   {
    case 1:    // lowpass
      for( n=1; n<=num_trans_samps; n++)
        {
         Des_Mag_Resp[N2-n] = origins[n] + rho * slopes[n];
        }
      break;
      
    case 2:    // highpass
      for( n=1; n<=num_trans_samps; n++)
        {
         Des_Mag_Resp[N1+n] = origins[n] + rho * slopes[n];
        }
      break;
                                             
    case 3:    // bandpass
      for( n=1; n<=num_trans_samps; n++)
        {
         Des_Mag_Resp[N1+n] = origins[n] + rho * slopes[n];
         Des_Mag_Resp[N4-n] = Des_Mag_Resp[N1+n];
        }
      break;
                                             
    case 4:    // bandstop
      for( n=1; n<=num_trans_samps; n++)
        {
         Des_Mag_Resp[N2-n] = origins[n] + rho * slopes[n];
         Des_Mag_Resp[N3+n] = Des_Mag_Resp[N2-n];
        }
      break;
   }
 return;
}
//==============================================================
void FreqSampFilterSpec::SetMagRespSamp( int samp_indx,
                                         double value)
{
 Des_Mag_Resp[samp_indx] = value;
 return;
}

logical FreqSampFilterSpec::IsOptimizeEnabled(void)
{
 return(Optimize_Enabled);
}
double FreqSampFilterSpec::GetMagRespSamp(int samp_indx)
{
 return( Des_Mag_Resp[samp_indx]);
}

int FreqSampFilterSpec::GetBandConfig(void)
{
return(Band_Config);
} 

int FreqSampFilterSpec::GetNumTaps(void)
{
return(Num_Taps);
}

int FreqSampFilterSpec::GetNumTransSamps(void)
{
return(Num_Trans_Samps);
}

int FreqSampFilterSpec::GetN1(void)
{
return(N1);
}
  
int FreqSampFilterSpec::GetN2(void)
{
return(N2);
}
  
int FreqSampFilterSpec::GetN3(void)
{
return(N3);
}
  
int FreqSampFilterSpec::GetN4(void)
{
return(N4);
}        

int FreqSampFilterSpec::GetFirType(void)
{
return(Fir_Type);
}

void FreqSampFilterSpec::SetBandConfig(int band_config)
{
 Band_Config = band_config;
} 

void FreqSampFilterSpec::SetNumTaps(int num_taps)
{
 Num_Taps = num_taps;
}

void FreqSampFilterSpec::SetN1(int n1)
{
 N1 = n1;
}
  
void FreqSampFilterSpec::SetN2(int n2)
{
 N2 = n2;
}
  
void FreqSampFilterSpec::SetN3(int n3)
{
 N3 = n3;
}
  
void FreqSampFilterSpec::SetN4(int n4)
{
 N4 = n4;
}        

void FreqSampFilterSpec::SetFirType(int fir_type)
{
 Fir_Type = fir_type;
}   

void FreqSampFilterSpec::SetDbScaleEnabled(int db_scale_enabled)
{
 Db_Scale_Enabled = db_scale_enabled;
}

 
