//
//  File = learncur.cpp
//

#include <iostream>
#include <fstream>
#include <stdlib.h>

#include "learncur.h"
#ifdef _DEBUG
extern std::ofstream DebugFile;
#endif

using namespace std;

LearningCurve::LearningCurve(int num_samps, int num_trials, double min_mse)
{
  Num_Samps = num_samps;
  Num_Trials = num_trials;
  Min_Mse = min_mse;
  Buffer = new double[Num_Samps];
  Sqr_Buf = new double[Num_Samps];
  Trial_Buf = new double[Num_Samps];
  for (int i = 0; i < num_samps; i++) {
    Buffer[i] = 0.0;
    Sqr_Buf[i] = 0.0;
    Trial_Buf[i] = 0.0;
  }
}
void
LearningCurve::AddSample(int samp_idx, double err_samp,
                         LOGICAL_T save_this_trial)
{
  double sqr_err;
  sqr_err = err_samp * err_samp;
  Buffer[samp_idx] += sqr_err;
  Sqr_Buf[samp_idx] += sqr_err * sqr_err;
  if (save_this_trial == _TRUE)
    Trial_Buf[samp_idx] = sqr_err;
}
void
LearningCurve::DumpCurve(void)
{
  double mean;
  ofstream out_file("learncur.txt", ios::out);
  for (int i = 0; i < Num_Samps; i++) {
    mean = Buffer[i] / Num_Trials;
    mean = Buffer[i] / Num_Trials;

    out_file << i << ", " << mean << ", " << Trial_Buf[i] << ", " << Min_Mse
             << std::endl;
  }
  out_file.close();
}
