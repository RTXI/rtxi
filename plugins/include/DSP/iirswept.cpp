//
//  File = iirswept.cpp
//

#include <fstream>
#include <math.h> 
#include <stdlib.h>
#include "iirswept.h"
#include "anlg_rcn.h"
#include "typedefs.h"
#include "misdefs.h"

extern std::ofstream AnalogWaveFile;
ofstream DigitalOutputFile("digital.txt",ios::out);


SweptResponse::SweptResponse( FilterImplementation *filter_implem,
                              double sampling_interval,
                              istream& uin,
                              ostream& uout )
{
 int resp_indx;
 double lambda, phase_lag, old_phase_lag;
 double input_val;
 double *output_tone;
 int samp_indx;
 int num_holdoff_samps;
 logical default_file_ok;
 Filter_Implem = filter_implem;

 int phase_indx;
 int num_worsening_phases, max_worsening_phases;
 int anlg_intrp_rate, num_sidelobes, num_anlg_samps;
 int max_num_samps;
 int samps_per_corr;
 double cycles_per_corr;
 double max_correl, phase_offset;
 double correl_coeff, norm_freq;
 double phase_delta, peak_mag;
 AnalogReconst *reconst_output;
 
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

 uout << "phase resolution?\n"
      << "  (in degrees)" << std::endl;
 uin >> phase_delta;
 if(phase_delta > 0.0) phase_delta = -phase_delta;

 uout << "number of worsening phases allowed\n"
      << "before aborting search and accepting\n"
      << "best found up to that point?" << std::endl;
 uin >> max_worsening_phases;

 uout << "numb sinewave cycles per correlation?" << std::endl;
 uin >> cycles_per_corr;
  
 if( Db_Scale_Enabled != 0) Db_Scale_Enabled = 1;

 uout << "analog interpolation rate (integer)?" << std::endl;
 uin >> anlg_intrp_rate;

 uout << "number of signif. sidelobes for sinc interp." << std::endl;
 uin >> num_sidelobes;
  
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
 Phase_Resp = new double[Num_Resp_Pts];
 max_num_samps = int(3*cycles_per_corr*Num_Resp_Pts/
                 (Max_Sweep_Freq * sampling_interval));
 output_tone = new double[max_num_samps+2]; 
 old_phase_lag = 0.0;
 for( resp_indx=1; resp_indx<Num_Resp_Pts; resp_indx++)
   {
    lambda = TWO_PI * resp_indx * Max_Sweep_Freq * 
              sampling_interval / (double) Num_Resp_Pts;
    norm_freq = resp_indx * Max_Sweep_Freq / (double) Num_Resp_Pts;
    samps_per_corr = int(Num_Resp_Pts*cycles_per_corr/
         (anlg_intrp_rate * Max_Sweep_Freq * sampling_interval));
    num_holdoff_samps = 2 * samps_per_corr;

    num_anlg_samps = anlg_intrp_rate * samps_per_corr;
    reconst_output = new AnalogReconst( sampling_interval,
                                        anlg_intrp_rate,
                                        num_sidelobes,
                                        num_anlg_samps );
    for( samp_indx=0; 
         samp_indx<(samps_per_corr+num_holdoff_samps);
         samp_indx++)
      {
       input_val = cos(lambda*samp_indx);
       output_tone[samp_indx] = 
                  filter_implem->ProcessSample(input_val);
       if(resp_indx == 6)
         {
         DigitalOutputFile << samp_indx << ", " << input_val << ", "
                           << output_tone[samp_indx] << std::endl;
         }
       if(samp_indx >= num_holdoff_samps)
         {
         reconst_output->AddSample( output_tone[samp_indx]);
         }
      }
    if(resp_indx == 6) 
      {
      reconst_output->DumpResult(&AnalogWaveFile);
      }

    // look for peak output magnitude
    peak_mag = 0.0;
    for( samp_indx=num_holdoff_samps; 
         samp_indx<(samps_per_corr+num_holdoff_samps);
         samp_indx++)
      {
        if( fabs(output_tone[samp_indx]) > peak_mag)
          peak_mag = fabs(output_tone[samp_indx]);
      }
    peak_mag = reconst_output->FindPeakMag();

   //===============================================
   // Create sinusoids in phase increments of 
   // phase_delta degrees and correlate them 
   // with the stored output tone.  Phase of sine
   // with maximum correlation will be taken as
   // phase response at that frequency.

   max_correl = 0.0;
   num_worsening_phases = 0;
   for( phase_indx = -max_worsening_phases; 
        phase_indx < int(-360./phase_delta);
        phase_indx++)
     {
      num_worsening_phases++;
      phase_offset = (old_phase_lag + 
                  (phase_indx * phase_delta)) * PI /180.0;
      correl_coeff = reconst_output->CosineCorrelate( norm_freq, 
                                                      phase_offset,
                                                      cycles_per_corr);
      /*
      sum = 0.0;
      auto_sum = 0.0;
    
      for( samp_indx=num_holdoff_samps; 
           samp_indx<(samps_per_corr+num_holdoff_samps);
           samp_indx++)
        {
         auto_sum += (cos(lambda*samp_indx + phase_offset)
                     *cos(lambda*samp_indx + phase_offset));
         sum += (output_tone[samp_indx]*
                cos(lambda*samp_indx + phase_offset));
        }
        */
      if(correl_coeff > max_correl)
        {
          max_correl = correl_coeff;
          phase_lag = double(old_phase_lag + phase_indx*phase_delta);
          num_worsening_phases = 0;
        }
      if(num_worsening_phases > max_worsening_phases) break;
     }
   //---------------------------------------
   // "unwrap" phase to keep it all negative

   old_phase_lag = phase_lag
                   - max_worsening_phases * phase_delta;
   while(phase_lag >= 180.0)
     { phase_lag -= 360.0; }
   //while(phase_lag <= -360.0)
   //  { phase_lag += 360.0; }

   Phase_Resp[resp_indx] = phase_lag;
   if(Db_Scale_Enabled)
     {
     Mag_Resp[resp_indx] = 
       20.0 * log10(max_correl);
     //Mag_Resp[resp_indx] = 
     //     20.0 * log10(peak_mag);
     }
   else
     {
     //Mag_Resp[resp_indx] = peak_mag;
     Mag_Resp[resp_indx] = max_correl;
     }
    delete reconst_output;
   }
 //if(Normalize_Enabled) NormalizeResponse();
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
 
 //Response_File->setf(ios::fixed, ios::floatfield);
 for(int n=1; n<Num_Resp_Pts; n++)
   {
    freq = n * Max_Sweep_Freq / (double) Num_Resp_Pts;
    (*Response_File) << freq << ", " 
                     << Mag_Resp[n] << ", "
                     << Phase_Resp[n] << std::endl;
   }
 //Response_File->setf(0, ios::floatfield);
 return;
}
