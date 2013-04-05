//
//  File = iir_resp.cpp
//
//  Member functions for class IirFilterResponse
//

#include <math.h>
#include <stdlib.h>
#include "complex.h"
#include "iir_resp.h"
#include "typedefs.h"
#include "misdefs.h"
#include "unwrap.h"
#ifdef _DEBUG
extern std::ofstream DebugFile;
#endif
//==================================================
//  constructor with all configuration parameters
//  passed in as arguments
//--------------------------------------------------

IirFilterResponse::IirFilterResponse( IirFilterDesign *filter_design,
                                      int num_resp_pts,
                                      int db_scale_enabled,
                                      int normalize_enabled,
                                      char* resp_file_name )
{
 Filter_Design = filter_design;
 Num_Resp_Pts = num_resp_pts;
 Db_Scale_Enabled = db_scale_enabled;
 Normalize_Enabled = normalize_enabled;
 
 if( resp_file_name == NULL)
   { Response_File = new ofstream("iir_resp.txt", ios::out);}
 else
   { Response_File = new ofstream(resp_file_name, ios::out);}
   
 Num_Numer_Coeffs = Filter_Design->GetNumNumerCoeffs();
 Num_Denom_Coeffs = Filter_Design->GetNumDenomCoeffs();
 Freq_Resp = (complex*) new double[2*Num_Resp_Pts];
 Mag_Resp = new double[Num_Resp_Pts];
 Phase_Resp = new double[Num_Resp_Pts];

 return;
}

//===================================================================
// alternate constructor with interactive setting of
// configuration parameters
//-------------------------------------------------------------------
IirFilterResponse::IirFilterResponse( IirFilterDesign *filter_design,
                                      istream& uin,
                                      ostream& uout )
{
 logical default_file_ok;
 Filter_Design = filter_design;
 
 uout << "number of points in plot of frequency response?" << std::endl;
 uin >> Num_Resp_Pts;
 
 uout << "scaling?\n"
      << "  0 = linear, 1 = dB"  << std::endl;
 uin >> Db_Scale_Enabled;
  
 if( Db_Scale_Enabled != 0) Db_Scale_Enabled = 1;
 Normalize_Enabled = 1;
  
 uout << "default name for magnitude response output\n"
      << "file is iir_resp.txt\n\n"
      << "is this okay?"
      << "  0 = NO, 1 = YES"
      << std::endl;
 uin >> default_file_ok;
  
 if( default_file_ok)
    {
     Response_File = new ofstream("iir_resp.txt", ios::out);
    }
  else
    {
     char *file_name;
     file_name = new char[31];
     
     uout << "enter complete name for output file (30 chars max)"
          << std::endl;
     uin >> file_name;
     Response_File = new ofstream(file_name, ios::out);
     delete []file_name;
    } 

 Num_Numer_Coeffs = Filter_Design->GetNumNumerCoeffs();
 Num_Denom_Coeffs = Filter_Design->GetNumDenomCoeffs();
 Freq_Resp = (complex*)new double[2*Num_Resp_Pts];
 Mag_Resp = new double[Num_Resp_Pts];
 Phase_Resp = new double[Num_Resp_Pts];

 return;
} 


//==================================================
//  method to compute frequency response
//--------------------------------------------------
void IirFilterResponse::ComputeResponse( void )
{
 int resp_indx, tap_indx;
 double delta_time, delta_freq;
 double theta, phi;
 double real_sum, imag_sum;
 double *numer_coeff, *denom_coeff;
 complex numerator, denominator;
 
 std::cout << " in IirFilterResponse::ComputeResponse" << std::endl;
 #ifdef _DEBUG
 DebugFile << " in IirFilterResponse::ComputeResponse" 
           << std::endl;
 #endif
 numer_coeff = Filter_Design->GetNumerCoefficients();
 denom_coeff = Filter_Design->GetDenomCoefficients();
 delta_time = Filter_Design->GetSamplingInterval();

 delta_freq = PI/( delta_time * Num_Resp_Pts );
 theta = delta_freq * delta_time;

 #ifdef _DEBUG
 DebugFile << "in ComputeResponse, Num_Numer_Coeffs = " 
           << Num_Numer_Coeffs << std::endl;
 DebugFile << "in ComputeResponse, Num_Denom_Coeffs = " 
           << Num_Denom_Coeffs << std::endl;
 #endif

 for( resp_indx=0; resp_indx<Num_Resp_Pts; resp_indx++)
   {
    real_sum = 0.0;
    imag_sum = 0.0;

    for( tap_indx=0; tap_indx<Num_Numer_Coeffs; tap_indx++)
     {
      phi = theta * resp_indx * tap_indx;
      real_sum += (numer_coeff[tap_indx] * cos(phi));
      imag_sum -= numer_coeff[tap_indx] * sin(phi);
     }  
    numerator = complex(real_sum, imag_sum);

    real_sum = 1.0;
    imag_sum = 0.0;
      
    for( tap_indx=1; tap_indx<=Num_Denom_Coeffs; tap_indx++)
      {
       phi = theta * tap_indx * resp_indx;
       real_sum -= (denom_coeff[tap_indx] * cos(phi));
       imag_sum += (denom_coeff[tap_indx] * sin(phi));
      } 
    denominator = complex(real_sum, imag_sum); 
    Freq_Resp[resp_indx] = numerator/denominator;
   }
   
 //-----------------------------------------------
 //  compute magnitude and phase of response

 for( resp_indx=0; resp_indx<Num_Resp_Pts; resp_indx++)
   {
    Phase_Resp[resp_indx] = arg(Freq_Resp[resp_indx]);

   if(Db_Scale_Enabled)
     {Mag_Resp[resp_indx] = 10.0 * log10(mag_sqrd(Freq_Resp[resp_indx]));}
   else
     {Mag_Resp[resp_indx] = norm(Freq_Resp[resp_indx]);}
   }
 

 return;
}

//=======================================================
//  method to normalize magnitude response
//-------------------------------------------------------

void IirFilterResponse::NormalizeResponse( void )
{
 int n;
 double biggest;
 
 if(Db_Scale_Enabled)
   {
    biggest = -100.0; 
    
    for( n=0; n < Num_Resp_Pts; n++)
      {if(Mag_Resp[n]>biggest) biggest = Mag_Resp[n];}
    for( n=0; n < Num_Resp_Pts; n++)
      {Mag_Resp[n] = Mag_Resp[n] - biggest;}
   }
 else
   {
    biggest = 0.0;
    
    for( n=0; n < Num_Resp_Pts; n++)
      {if(Mag_Resp[n]>biggest) biggest = Mag_Resp[n];}
    for( n=0; n < Num_Resp_Pts; n++)
      {Mag_Resp[n] = Mag_Resp[n] / biggest;}
   }
 return;
}
//===============================================
//  method to return a pointer to the magnitude
//  response that is stored inside this class
//-----------------------------------------------

double* IirFilterResponse::GetMagResp( void)
{
 return(Mag_Resp);
}

//===========================================================
//  method to dump magnitude response to the stream
//  designated by Response_File
//-----------------------------------------------------------

void IirFilterResponse::DumpMagResp( void )
{
 double freq, delta_time, delta_freq, phase;
 
 delta_time = Filter_Design->GetSamplingInterval();
 delta_freq = 0.5/( delta_time * Num_Resp_Pts );

 for(int n=0; n<Num_Resp_Pts; n++)
   {
    freq = n*delta_freq;
    phase = 180.0 * Phase_Resp[n] /PI;
    UnwrapPhase(n, &phase);
    (*Response_File) << freq << ", " 
                     << Mag_Resp[n] << ", "
                     << phase << std::endl;
   }
 return;
}
//=====================================================
//  searches the magnitude response over the interval
//  from sample intrvl_beg thru the sample intrvl_end
//  and then returns the largest value found in this
//  interval.
//-----------------------------------------------------
double IirFilterResponse::GetIntervalPeak( int nBeg,
                                           int nEnd)
{
 double peak;
 int n, indexOfPeak;

 peak = -9999.0;
 for(n=nBeg; n<nEnd; n++)
   {
    if(Mag_Resp[n]>peak)
      {
       peak=Mag_Resp[n];
       indexOfPeak = n;
      }
   }
 return(peak);
}

