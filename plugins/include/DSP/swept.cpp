//
//  File = swept.cpp
//

#include <fstream>
#include <math.h> 
#include <stdlib.h>
#include "swept.h"
#include "typedefs.h"
#include "misdefs.h"
#ifdef _DEBUG
extern std::ofstream DebugFile;
#endif

SweptResponse::SweptResponse( FilterImplementation *filter_implem,
                              double sampling_interval,
                              istream& uin,
                              ostream& uout )
{
 int resp_indx;
 double lambda;
 double input_val;
 double *output_tone;
 int samp_indx;
 int num_holdoff_samps;
 logical default_file_ok;
 Filter_Implem = filter_implem;

 int max_num_samps;
 int samps_per_corr;
 double cycles_per_corr;
 double max_output_mag;
 
 uout << "number of points in plot of frequency response?" << std::endl;
 uin >> Num_Resp_Pts;

 uout << "maximum swept frequency?" << std::endl;
 uin >> Max_Sweep_Freq;
 if(Max_Sweep_Freq > (0.5/sampling_interval) )
   {
    uout << "maximum swept frequency will be limited\n"
         << "to folding frequency of " 
         << (0.5/sampling_interval) << std::endl;
    Max_Sweep_Freq = 0.5/sampling_interval;
   }
 
 uout << "scaling?\n"
      << "  0 = linear, 1 = dB"  << std::endl;
 uin >> Db_Scale_Enabled;

 uout << "number of output cycles to examine?" << std::endl;
 uin >> cycles_per_corr;
  
 if( Db_Scale_Enabled != 0) Db_Scale_Enabled = 1;
  
 uout << "default name for magnitude response output\n"
      << "file is win_resp.txt\n\n"
      << "is this okay?"
      << "  0 = NO, 1 = YES"
      << std::endl;
 uin >> default_file_ok;
  
 if( default_file_ok) 
    {
     Response_File = new ofstream("win_resp.txt", ios::out);
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
 Mag_Resp = new double[Num_Resp_Pts];
 max_num_samps = int(2*cycles_per_corr*Num_Resp_Pts/
                 (Max_Sweep_Freq * sampling_interval));
 output_tone = new double[max_num_samps+2]; 
 for( resp_indx=1; resp_indx<Num_Resp_Pts; resp_indx++)
   {
    lambda = resp_indx * Max_Sweep_Freq * 2.0 * PI * 
              sampling_interval / (double) Num_Resp_Pts;
    samps_per_corr = int(Num_Resp_Pts*cycles_per_corr/
         (resp_indx * Max_Sweep_Freq * sampling_interval));
    num_holdoff_samps = samps_per_corr;
    //num_holdoff_samps = int(Num_Resp_Pts/
    //     (resp_indx * Max_Sweep_Freq * sampling_interval));
    for( samp_indx=0; samp_indx<num_holdoff_samps;
         samp_indx++)
      {
       input_val = cos(lambda*samp_indx);
       output_tone[samp_indx] = 
                  filter_implem->ProcessSample(input_val);
      }
    max_output_mag = 0.0;
    for( samp_indx=num_holdoff_samps; 
         samp_indx<(samps_per_corr+num_holdoff_samps);
         samp_indx++)
      {
       input_val = cos(lambda*samp_indx);
       output_tone[samp_indx] = 
                  filter_implem->ProcessSample(input_val);
       if(fabs(output_tone[samp_indx]) > max_output_mag)
         {
         max_output_mag = fabs(output_tone[samp_indx]);
         }
      } 
    if(Db_Scale_Enabled)
      {
      Mag_Resp[resp_indx] = 
          20.0 * log10(max_output_mag);
      }
    else
      {Mag_Resp[resp_indx] = max_output_mag;}
    }  // end of loop over resp_indx

 if(Normalize_Enabled) NormalizeResponse();
 return;
}
//=======================================================
// destructor
//-------------------------------------------------------

SweptResponse::~SweptResponse()
{
 delete []Mag_Resp;
 delete Response_File;
}
//=======================================================
//  method to normalize magnitude response
//-------------------------------------------------------

void SweptResponse::NormalizeResponse( void )
{
 int n;
 double biggest;
 
 if(Db_Scale_Enabled)
   {
    biggest = -100.0; 
    
    for( n=1; n < Num_Resp_Pts; n++)
      {if(Mag_Resp[n]>biggest) biggest = Mag_Resp[n];}
    #ifdef _DEBUG
    DebugFile << "before normaliz, biggest Mag_Resp was "
              << biggest << std::endl;
    #endif
    for( n=1; n < Num_Resp_Pts; n++)
      {Mag_Resp[n] = Mag_Resp[n] - biggest;}
   }
 else
   {
    biggest = 0.0;
    
    for( n=1; n < Num_Resp_Pts; n++)
      {if(Mag_Resp[n]>biggest) biggest = Mag_Resp[n];}
    for( n=1; n < Num_Resp_Pts; n++)
      {Mag_Resp[n] = Mag_Resp[n] / biggest;}
   }
 return;
}
//===========================================================
//  method to dump magnitude response to the stream
//  designated by Response_File
//-----------------------------------------------------------

void SweptResponse::DumpMagResp( void )
{
 double freq;
 
 //Response_File->setf(ios::fixed, ios::doublefield);
 for(int n=1; n<Num_Resp_Pts; n++)
   {
    freq = n * Max_Sweep_Freq / (double) Num_Resp_Pts;
    (*Response_File) << freq << ", " 
                     << Mag_Resp[n] << std::endl;
   }
 //Response_File->setf(0, ios::doublefield);
 return;
}
