//
// File = remezalg.h
//
#ifndef _REMEZALG_H_
#define _REMEZALG_H_ 

#include "fir_dsgn.h" 

class RemezAlgorithm
{
public: 

  //  constructor
  RemezAlgorithm( std::istream& uin,
                  std::ostream& uout,
                  int filter_length,
                  double passband_edge_freq,
                  double stopband_edge_freq,
                  double ripple_ratio,
                  FirFilterDesign **fir_filter);
void SetupGrid( void );

double GetFrequency( int grid_index); 

void RemezError( void); 

void RemezSearch(void);

int RemezStop(void);

int RemezStop2(void); 

double ComputeRemezAmplitudeResponse( int init_flag,
                                      double contin_freq); 
void RemezFinish(double *filter_coeffs);


private:

 int Num_PB_Freqs;
 int Num_SB_Freqs;
 int Num_Grid_Pts_PB;
 int Num_Grid_pts_SB;
 int Filter_Length;
 double PB_Increment;
 double SB_Increment;
 int Grid_Density;
 int Num_Approx_Funcs;
 double PB_Edge_Freq;
 double SB_Edge_Freq;
 int* Ext_Freq;
 int* Old_Ext_Freq;
 double* Error;
 int Max_Grid_Indx;
 double WeightFunction(double freq);
 double DesiredResponse(double freq);
 double Abs_Delta; 
 double Ripple_Ratio;
 double *X_;
 double *Beta_;
 double *Gamma_;
 
}; 
#endif 