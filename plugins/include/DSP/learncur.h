//
//  File = learncur.h
//

#ifndef _LEARNCUR_H_
#define _LEARNCUR_H_

#include "typedefs.h"
class LearningCurve
{
public:
  LearningCurve( int num_samps,
                 int num_trials,
                 double min_mse );
  void AddSample( int samp_idx,
                  double err_samp,
                  LOGICAL_T save_this_trial );
  void DumpCurve( void );
private:
  int Num_Samps;
  int Num_Trials;
  double Min_Mse;
  double *Buffer;
  double *Sqr_Buf;
  double *Trial_Buf;
};

#endif
