 //
 //  File = sinawgn.cpp
 //
 
 
 #include <stdlib.h>
 #include "sinawgn.h"
 #include "complex.h"
 #include "uni_rand.h"
 #include "gausrand.h"
 #include "misdefs.h"
 
 //==========================================
 // constructor with sine and noise parameters
 //             passed in call
 //------------------------------------------
 
 SinesInAwgn::SinesInAwgn( double samp_intvl,
                           int num_sines,
                           double *freq,
                           double *phase,
                           double *ampl,
                           double agn_sigma,
                           long agn_seed )
 {
  Samp_Intvl = samp_intvl;
  Num_Sines = num_sines;
  Freq = new double[num_sines];
  Phase = new double[num_sines];
  Ampl = new double[num_sines];
  for(int sin_idx=0; sin_idx<num_sines; sin_idx++)
    {
    Freq[sin_idx] = TWO_PI*freq[sin_idx];
    Phase[sin_idx] = TWO_PI*phase[sin_idx];
    Ampl[sin_idx] = ampl[sin_idx];
    }
  Agn_Sigma = agn_sigma;
  Orig_Agn_Seed = agn_seed;
  Agn_Seed = agn_seed;
  Seg_Offset = 0;
 }
  
 //==========================================
 // constructor with sine and noise parameters
 //             requested from user
 //------------------------------------------
 
 SinesInAwgn::SinesInAwgn( double samp_intvl,
                           istream &uin,
                           ostream &uout)
 {
  Samp_Intvl = samp_intvl;
  uout << "How many sinusoids?" << std::endl;
  uin >> Num_Sines;
  Freq = new double[Num_Sines];
  Phase = new double[Num_Sines];
  Ampl = new double[Num_Sines];
  for(int sin_idx=0; sin_idx<Num_Sines; sin_idx++)
    {
    uout << "Desired frequency of sinusoid #" << sin_idx
         << " ?" << std::endl;
    uin >> (Freq[sin_idx]);
    Freq[sin_idx] *= TWO_PI;
    uout << "Desired phase of sinusoid #" << sin_idx
         << " ?" << std::endl;
    uin >> (Phase[sin_idx]);
    Phase[sin_idx] *= TWO_PI;
    uout << "Desired amplitude of sinusoid #" << sin_idx
         << " ?" << std::endl;
    uin >> (Ampl[sin_idx]);
    }
  uout << "Sigma for noise?" << std::endl;
  uin >> Agn_Sigma;
  Orig_Agn_Seed = 4973;
  Agn_Seed = Orig_Agn_Seed;
  uout << "default noise seed is " << Agn_Seed << std::endl;
  Seg_Offset = 0;
 }
  
 //=============================================
 // reset source to original startup conditions
 //---------------------------------------------
 
 void SinesInAwgn::ResetSource( void )
 {
  Agn_Seed = Orig_Agn_Seed;
  Seg_Offset = 0;
 }
  
 //++++++++++++++++++++++++++++++++++++++++++
 
 
 //==========================================
 // destructor
 //------------------------------------------
 
 SinesInAwgn::~SinesInAwgn()
 {
 }
 //++++++++++++++++++++++++++++++++++++++++++
 
 
 //====================================================
 //  method to generate the next segment of signal data
 //----------------------------------------------------
 
 void SinesInAwgn::GetNextSegment( complex *time_seq,
                                   int num_samps )
 { 
  int samp_idx, sin_idx;
  double value;
  double time;
    
  for(samp_idx=0; samp_idx < num_samps; samp_idx++)
    {
     value = Agn_Sigma * GaussRandom(&Agn_Seed);
     time = (samp_idx + Seg_Offset) * Samp_Intvl;
     for(sin_idx=0; sin_idx<Num_Sines; sin_idx++)
       {
       value += Ampl[sin_idx] * 
           sin( Freq[sin_idx] * time + Phase[sin_idx]);
       }
    time_seq[samp_idx] = value;
    }
  Seg_Offset += num_samps;
  return;
 }