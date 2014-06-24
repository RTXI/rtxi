//
//  File = adap_fir.cpp
//

#include <stdlib.h>
#include <fstream>
#include <math.h>
#include "dirform1.h"
#include "adap_fir.h"
#ifdef _DEBUG
extern std::ofstream DebugFile;
#endif

AdaptiveFir::AdaptiveFir( int num_taps,
                          double *coeff,
                          logical quan_enab,
                          long coeff_quan_factor,
                          long input_quan_factor,
                          int tap_for_trans,
                          int secondary_tap,
                          int transient_len)
            :DirectFormFir( num_taps,
                            coeff,
                            quan_enab,
                            coeff_quan_factor,
                            input_quan_factor)
{
 int samp_idx;
 Update_Count = 0;
 Trial_Count = 0;
 Tap_For_Trans = tap_for_trans;
 Secondary_Tap = secondary_tap;
 Transient_Len = transient_len;
 Tally_For_Avg = new double[transient_len];
 Tally_For_Avg_2 = new double[transient_len];
 Sample_Transient = new double[transient_len];
 Sample_Trans_2 = new double[transient_len];
 for(samp_idx=0; samp_idx<transient_len; samp_idx++)
 {
   Tally_For_Avg[samp_idx] = 0.0;
   Tally_For_Avg_2[samp_idx] = 0.0;
   Sample_Transient[samp_idx] = 0.0;
   Sample_Trans_2[samp_idx] = 0.0;
 }
 #ifdef _DEBUG
 DebugFile << "In AdaptiveFir" << std::endl;
 #endif
 return;
}

void AdaptiveFir::DumpAvgTransient( int decim_factor )
{
  int samp_idx;

  ofstream trans_file("avg_tran.txt", ios::out);
  for(samp_idx=0; samp_idx<Transient_Len; samp_idx+=decim_factor)
    {
    trans_file << samp_idx << ", "
               << (Tally_For_Avg[samp_idx]/double(Trial_Count))
               << ", " << Sample_Transient[samp_idx]
               << std::endl;
    }
  trans_file.close();
}
void AdaptiveFir::DumpTrajectory( double min_dist )
{
  int samp_idx;
  double x, y, old_x, old_y, dist;
  ofstream trans_file("traject.txt", ios::out);

  old_x = Sample_Transient[0];
  old_y = Sample_Trans_2[0];
  trans_file << (0) << ", " << old_x << ", " << old_y << std::endl;

  for(samp_idx=1; samp_idx<Transient_Len; samp_idx++)
    {
    x = Sample_Transient[samp_idx];
    y = Sample_Trans_2[samp_idx];
    dist = sqrt((old_x-x)*(old_x-x) + (old_y-y)*(old_y-y));
    if(dist < min_dist) continue;
    trans_file << samp_idx << ", " << x << ", " << y << std::endl;
    old_x = x;
    old_y = y;
    }
  trans_file.close();
}
void AdaptiveFir::DumpAvgTrajectory( double min_dist )
{
  int samp_idx;
  double x, y, old_x, old_y, dist;
  ofstream trans_file("avg_traj.txt", ios::out);

  old_x = Tally_For_Avg[0]/double(Trial_Count);
  old_y = Tally_For_Avg_2[0]/double(Trial_Count);
  trans_file << (0) << ", " << old_x << ", " << old_y << std::endl;

  for(samp_idx=1; samp_idx<Transient_Len; samp_idx++)
    {
    x = Tally_For_Avg[samp_idx]/double(Trial_Count);
    y = Tally_For_Avg_2[samp_idx]/double(Trial_Count);
    dist = sqrt((old_x-x)*(old_x-x) + (old_y-y)*(old_y-y));
    if(dist < min_dist) continue;
    trans_file << samp_idx << ", " << x << ", " << y << std::endl;
    old_x = x;
    old_y = y;
    }
  trans_file.close();
}
void AdaptiveFir::DumpTransient( int decim_factor )
{
  int samp_idx;

  ofstream trans_file("samptran.txt", ios::out);

  for(samp_idx=0; samp_idx<Transient_Len; samp_idx+=decim_factor)
    {
    trans_file << samp_idx << ", "
               << Sample_Transient[samp_idx]
               << std::endl;
    }
  trans_file.close();
}
void AdaptiveFir::GetTaps(double *taps)
{
  int i;
  for(i=0; i<Num_Taps; i++)
    {
    taps[i] = Unquan_Coeff[i];
    }
}


