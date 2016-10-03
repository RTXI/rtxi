//
//  File = sinawgn.h
//

#ifndef _SINAWGN_H_
#define _SINAWGN_H_ 

 #include "complex.h"
 #include "sig_src.h"

 class SinesInAwgn : public SignalSource
 { 
 public:

 SinesInAwgn( double sampling_interval,
              std::istream &uin,
              std::ostream &uout);

  SinesInAwgn( double sampling_interval,
               int num_sines,
               double *freq,
               double *phase,
               double *ampl,
               double agn_sigma,
               long agn_seed );

  void ResetSource( void );

  ~SinesInAwgn(); 
  
//  float_complex* GetNextSegment(void);
 void GetNextSegment( complex *time_seq,
                      int num_samps );

 private:
 
  double Samp_Intvl;
  int Num_Sines;
  double *Freq; 
  double *Phase; 
  double *Ampl;
  double Agn_Sigma;
  long Agn_Seed;
  long Orig_Agn_Seed;
  int Seg_Offset;
  
 };
#endif // _SINAWGN_H_