//
//  File = cpfsk.h
//

#ifndef _CPFSK_H_
#define _CPFSK_H_ 
#include "complex.h"
#include "sig_src.h"
#include "typedefs.h"

 class CpfskSource : public SignalSource
 { 
 public:

 CpfskSource( double sampling_interval,
              double freq_dev,
              int samps_per_symb,
              long prn_seed,
              int num_samps_per_seg,
              int samps_initial_delay);

  ~CpfskSource(); 
  
//  float_complex* GetNextSegment(void);
  void GetNextSegment( double* output_vector,
                       int num_samps);

  void GetNextSegment( complex* output_vector,
                       int num_samps);

 private:
 
  double Samp_Intvl;
  int Num_Samps_Per_Seg;
  int Seg_Num;
  int Samps_Per_Symb;
  int Saved_Samp_Idx;
  long Prn_Seed;
  double Freq_Dev;
  double Old_Phi;
  double Saved_Rad_Freq;
  complex* Time_Signal;
  int Samps_Initial_Delay;
  logical In_Delay_Intvl;
  
 };
#endif // _CPFSK_H_