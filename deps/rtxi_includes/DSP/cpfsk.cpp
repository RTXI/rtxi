 //
 //  File = cpfsk.cpp
 //
 
 
 #include <stdlib.h>
 #include <iostream>
 #include <fstream>
 #include <math.h>
 #include "cpfsk.h"
 #include "complex.h"
 #include "misdefs.h"
 #include "uni_rand.h"
 
 //==========================================
 // constructor
 //------------------------------------------
 
 CpfskSource::CpfskSource( double samp_intvl,
                           double freq_dev,
                           int samps_per_symb,
                           long prn_seed,
                           int num_samps_per_seg,
                           int samps_initial_delay)
            : SignalSource()
 {
  Samp_Intvl = samp_intvl;
  Freq_Dev = freq_dev;
  Prn_Seed = prn_seed;
  Num_Samps_Per_Seg = num_samps_per_seg;
  Seg_Num = 0;
  Samps_Per_Symb = samps_per_symb;
  Samps_Initial_Delay = samps_initial_delay;

  if(samps_initial_delay == 0)
    In_Delay_Intvl = FALSE;
  else
    In_Delay_Intvl = TRUE;

  Old_Phi = 0.0;
  Saved_Samp_Idx = 0;
  Saved_Rad_Freq = 0.0;
 }
  
 //++++++++++++++++++++++++++++++++++++++++++
 
 
 //==========================================
 // destructor
 //------------------------------------------
 
 CpfskSource::~CpfskSource()
 {
 }
 //++++++++++++++++++++++++++++++++++++++++++
 
 
 //====================================================
 //  method to generate the next segment of signal data
 //----------------------------------------------------
 
 void CpfskSource::GetNextSegment( double *time_seq,
                                   int num_samps )
 { 
  int samp_idx, cumul_samp_idx;
  double phi, rand_val, rad_freq;
  double td_energy;

  cumul_samp_idx = 0;
  td_energy = 0.0;

  if(In_Delay_Intvl == TRUE)
    {
    for(samp_idx=0; samp_idx<Samps_Initial_Delay; samp_idx++)
      {
      time_seq[cumul_samp_idx] = 0.0;
      cumul_samp_idx++;
      }
    In_Delay_Intvl = FALSE;
    }

   while(cumul_samp_idx < num_samps)
    {
    if(Saved_Samp_Idx != 0)
      {
      for(samp_idx=Saved_Samp_Idx; samp_idx < Samps_Per_Symb; 
          samp_idx++)
        {
        phi = Old_Phi + Saved_Rad_Freq * Samp_Intvl;
        Old_Phi = fmod(phi, TWO_PI);
        time_seq[cumul_samp_idx] = sin(Old_Phi);
        td_energy += (sin(Old_Phi)*sin(Old_Phi));
        cumul_samp_idx++;
        }
      Saved_Samp_Idx = 0;
      }
    // flip a fair coin to get next symbol value
    rand_val = UniformRandom(&Prn_Seed);
    if(rand_val < 0.5) 
      rad_freq = -TWO_PI * Freq_Dev;
    else
      rad_freq = TWO_PI * Freq_Dev;

    // generate samples for next symbol
    for(samp_idx=0; samp_idx < Samps_Per_Symb; samp_idx++)
      {
      phi = Old_Phi + rad_freq * Samp_Intvl;
      Old_Phi = fmod(phi, TWO_PI);
      time_seq[cumul_samp_idx] = sin(Old_Phi);
      td_energy += (sin(Old_Phi)*sin(Old_Phi));
      cumul_samp_idx++;
      if(cumul_samp_idx >= num_samps) 
        {
        Saved_Rad_Freq = rad_freq;
        Saved_Samp_Idx = samp_idx+1;
        break;
        }
      } // end of loop over samp_idx
    }
  Seg_Num++;
  
  return;
 }
  //====================================================
 //  method to generate the next segment of signal data
 //----------------------------------------------------
 
 void CpfskSource::GetNextSegment( complex *time_seq,
                                   int num_samps )
 { 
  int samp_idx, cumul_samp_idx;
  double phi, rand_val, rad_freq;
  double td_energy;

  cumul_samp_idx = 0;
  td_energy = 0.0;

  if(In_Delay_Intvl == TRUE)
    {
    for(samp_idx=0; samp_idx<Samps_Initial_Delay; samp_idx++)
      {
      time_seq[cumul_samp_idx] = 0.0;
      cumul_samp_idx++;
      }
    In_Delay_Intvl = FALSE;
    }

   while(cumul_samp_idx < num_samps)
    {
    // Finish off any partial symbol started in previous segment
    if(Saved_Samp_Idx != 0)
      {
      for(samp_idx=Saved_Samp_Idx; samp_idx < Samps_Per_Symb; 
          samp_idx++)
        {
        phi = Old_Phi + Saved_Rad_Freq * Samp_Intvl;
        Old_Phi = fmod(phi, TWO_PI);
        time_seq[cumul_samp_idx] = complex(sin( Old_Phi ), 0.0);
        td_energy += (sin(Old_Phi)*sin(Old_Phi));
        cumul_samp_idx++;
        }
      Saved_Samp_Idx = 0;
      }
    // flip a fair coin to get next symbol value
    rand_val = UniformRandom(&Prn_Seed);
    if(rand_val < 0.5) 
      rad_freq = -TWO_PI * Freq_Dev;
    else
      rad_freq = TWO_PI * Freq_Dev;

    // generate samples for next symbol
    for(samp_idx=0; samp_idx < Samps_Per_Symb; samp_idx++)
      {
      phi = Old_Phi + rad_freq * Samp_Intvl;
      Old_Phi = fmod(phi, TWO_PI);
      time_seq[cumul_samp_idx] = complex(sin( Old_Phi ), 0.0);
      td_energy += (sin(Old_Phi)*sin(Old_Phi));
      cumul_samp_idx++;
      if(cumul_samp_idx >= num_samps) 
        {
        Saved_Rad_Freq = rad_freq;
        Saved_Samp_Idx = samp_idx+1;
        break;
        }
      } // end of loop over samp_idx
    }
  if(Seg_Num==0) std::cout << "td_energy = " << td_energy << std::endl;
  Seg_Num++;
  
  return;
 }