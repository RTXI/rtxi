//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  File = remezalg.cpp
//
//  Remez algorithm for design of FIR filters
//

#include <math.h>
#include <stdlib.h>
#include <ostream.h>
#include "fs_dsgn.h"
#include "fs_spec.h"
#include "remezalg.h"
#ifdef _DEBUG
extern std::ofstream DebugFile;
#endif

//======================================================
//  constructor
//------------------------------------------------------

RemezAlgorithm::RemezAlgorithm( istream& uin,
                                ostream& uout,
                                int filter_length,
                                double passband_edge_freq,
                                double stopband_edge_freq,
                                double ripple_ratio,
                                FirFilterDesign **fir_filter)
{
 int m, j;
 int grid_density;
 //--------------------------------
 // get user-specified parameters

  uout << "\nparameters for Remez algorithm\n"
       << "--------------------------------" << std::endl;
  uout << "number of grid points per extremal frequency?\n"
       << "( MUST be an integer)" << std::endl;
  uin >> grid_density;

  //uout << "ripple ratio?" << std::endl;
  //uin >> ripple_ratio;

 //--------------------------------
 //  set up frequency grid

 Filter_Length = filter_length;
 PB_Edge_Freq = passband_edge_freq;
 SB_Edge_Freq = stopband_edge_freq;
 Num_Approx_Funcs = (filter_length + 1)/2;
 Grid_Density = grid_density;
 Ripple_Ratio = ripple_ratio;

 double* extremal_freqs = new double[Num_Approx_Funcs+1];
 double* filter_coeffs = new double[filter_length+1];
 
 SetupGrid();

 Ext_Freq = new int[Num_PB_Freqs+Num_SB_Freqs];
 Old_Ext_Freq = new int[Num_PB_Freqs+Num_SB_Freqs];
 
 PB_Edge_Freq = PB_Edge_Freq + (PB_Edge_Freq/(2.0*Num_Grid_Pts_PB));
 Max_Grid_Indx = 1 + Grid_Density*(Num_PB_Freqs+Num_SB_Freqs-1);
 Error = new double[Max_Grid_Indx+1];

 //----------------------------------------------
 //  make initial guess of extremal frequencies           

 for(j=0; j<Num_PB_Freqs; j++) Ext_Freq[j] = (j+1)* grid_density;

 for(j=0; j<Num_SB_Freqs; j++) Ext_Freq[j+Num_PB_Freqs] = 
                               Num_Grid_Pts_PB + 1 + j * grid_density;

 //----------------------------------------------------
 //  find optimal locations for extremal frequencies 

 for(m=1;m<=20;m++) 
   {
    RemezError();

    RemezSearch();
  
    RemezStop2();
    if(RemezStop()) break;
    #ifdef _DEBUG
    DebugFile << "done iteration " << m << std::endl;
    #endif
  }
   
 for(j=0; j<=Num_Approx_Funcs; j++)
   {
    extremal_freqs[j] = GetFrequency(Ext_Freq[j]);
    #ifdef _DEBUG
    DebugFile << "extremal_freqs[ " << j << " ] = "
              << extremal_freqs[j] << std::endl;
    #endif
   }

 RemezFinish( filter_coeffs);

 *fir_filter = new FirFilterDesign( filter_length, 
                                    FIR_SYM_EVEN_LEFT, 
                                    filter_coeffs);
 return;
};
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++


void RemezAlgorithm::RemezFinish( double *filter_coeffs )
{
 int k;
 double freq, *aa;
  FreqSampFilterSpec *filter_spec;
  FreqSampFilterDesign *filter_design; 
 
 aa = new double[Num_Approx_Funcs];
 
 for( k=0; k<Num_Approx_Funcs; k++)
   {
    freq = (double) k/ (double) Filter_Length;
    aa[k] = ComputeRemezAmplitudeResponse (0, freq);
   }
 filter_spec = new FreqSampFilterSpec( 1, 1, Filter_Length, aa); 
 filter_design = new FreqSampFilterDesign( *filter_spec);
 filter_design->ComputeCoefficients( filter_spec);
 filter_design->CopyCoefficients( filter_coeffs);
 
 
 for(k=0; k<Filter_Length; k++)
   {
    #ifdef _DEBUG
    DebugFile << "Coeff[ " << k << " ] = "
              << filter_coeffs[k] << std::endl;
    #endif
   }
}
    


//======================================================
//
//------------------------------------------------------

void RemezAlgorithm::SetupGrid( void )
{
 double work;
 work = (0.5 + PB_Edge_Freq - SB_Edge_Freq)/Num_Approx_Funcs;
 
 Num_PB_Freqs = (int)floor(0.5 + PB_Edge_Freq/work);
  
 Num_Grid_Pts_PB = Num_PB_Freqs * Grid_Density;

 Num_SB_Freqs = Num_Approx_Funcs + 1 - Num_PB_Freqs;

 PB_Increment = PB_Edge_Freq / Num_Grid_Pts_PB;
 
 SB_Increment = ( 0.5 - SB_Edge_Freq )/((Num_SB_Freqs-1) * Grid_Density);
 return;
} 
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

double RemezAlgorithm::GetFrequency( int grid_index)
{ 
 if( grid_index <= Num_Grid_Pts_PB )
   {
    // compute freq in passband
    
    return( grid_index * PB_Increment );
   }
 else
   {
    // compute freq in stopband
    
    return( SB_Edge_Freq + 
          ( grid_index - (Num_Grid_Pts_PB+1))*SB_Increment);
   }   
} 
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ 

void RemezAlgorithm::RemezError( void)
{
 int j;
 double freq;
 double ampl_resp;
 
 ampl_resp = ComputeRemezAmplitudeResponse( 1, 0.0);
 
 for( j=0; j<= Max_Grid_Indx; j++)
   {
    freq = GetFrequency(j);
    
    ampl_resp = ComputeRemezAmplitudeResponse( 0, freq);
    
    Error[j] = WeightFunction(freq) * (DesiredResponse(freq) - ampl_resp);
   } 
 return;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

double RemezAlgorithm::ComputeRemezAmplitudeResponse( 
                                               int init_flag,
                                               double contin_freq)
{
 static int i, j, k, sign;
 static double freq, denom, numer, alpha, delta;
 static double absDelta, xCont, term;
 double aa;

 if(init_flag)
   {
    X_ = new double[Num_Approx_Funcs+1];
    Beta_ = new double[Num_Approx_Funcs+1];
    Gamma_ = new double[Num_Approx_Funcs+1];
    for(j=0; j<=Num_Approx_Funcs; j++)
      {
       freq = GetFrequency(Ext_Freq[j]);
       X_[j] = cos(TWO_PI * freq);
      }
  
    //  compute delta
    denom = 0.0;
    numer = 0.0;
    sign = -1;
    for( k=0; k<=Num_Approx_Funcs; k++)
      {
       sign = -sign;
       alpha = 1.0;
       for( i=0; i<=(Num_Approx_Funcs-1); i++)
         {
          if(i==k) continue;
            alpha = alpha / (X_[k] - X_[i]);
         }
       Beta_[k] = alpha;
       if( k != Num_Approx_Funcs ) 
                         alpha = alpha/(X_[k] - X_[Num_Approx_Funcs]);
       freq =  GetFrequency(Ext_Freq[k]);
       numer = numer + alpha * DesiredResponse(freq);
                 
       denom = denom + sign*(alpha/WeightFunction(freq));
      } // end of loop over k
    
    delta = numer/denom;
    absDelta = fabs(delta);
  
    sign = -1;
    for( k=0; k<=Num_Approx_Funcs-1; k++)
      {
       sign = -sign;
       freq = GetFrequency(Ext_Freq[k]);
       Gamma_[k] = DesiredResponse(freq) - sign * delta / 
            WeightFunction(freq);
      }
   } // end of if(init_flag)
 else
   {
    xCont = cos(TWO_PI * contin_freq);
    numer = 0.0;
    denom = 0.0;
    for( k=0; k<Num_Approx_Funcs; k++)
      {
       term = xCont - X_[k];
       if(fabs(term)<1.0e-7)
         {
          aa = Gamma_[k];
          goto done;
         }
       else
         {
          term = Beta_[k]/(xCont - X_[k]);
          denom += term;
          numer += Gamma_[k]*term;
         }
      }
    aa = numer/denom;
   }
 done:
 return(aa); 
}
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void RemezAlgorithm::RemezSearch(void)
{
 int i,j,k,extras,indexOfSmallest;
 double minVal;

 k=0;

 /* test for extremum at f=0  */
 if( ( (Error[0]>0.0) && 
       (Error[0]>Error[1]) && 
       (fabs(Error[0])>=Abs_Delta) ) ||
     ( (Error[0]<0.0) && 
       (Error[0]<Error[1]) && 
       (fabs(Error[0])>=Abs_Delta) ) )
   { 
    Ext_Freq[k]=0;
    k++;
   }

 /*  search for extrema in passband  */
 for(j=1; j<Num_Grid_Pts_PB; j++)
   {
    if( ( (Error[j]>=Error[j-1]) && 
          (Error[j]>Error[j+1]) && 
          (Error[j]>0.0) ) ||
        ( (Error[j]<=Error[j-1]) && 
          (Error[j]<Error[j+1]) && (Error[j]<0.0) ))
      {
       Ext_Freq[k] = j;
       k++;
      }
   }

 /* pick up an extremal frequency at passband edge  */
 Ext_Freq[k]=Num_Grid_Pts_PB;
 k++;

 /* pick up an extremal frequency at stopband edge  */
 j=Num_Grid_Pts_PB+1;
 Ext_Freq[k]=j;
 k++;

 /*  search for extrema in stopband  */
     
 for(j=Num_Grid_Pts_PB+2; j<Max_Grid_Indx; j++)
   {
    if( ( (Error[j]>=Error[j-1]) && 
          (Error[j]>Error[j+1]) && 
          (Error[j]>0.0) ) ||
        ( (Error[j]<=Error[j-1]) && 
          (Error[j]<Error[j+1]) && 
          (Error[j]<0.0) ))
      {
       Ext_Freq[k] = j;
       k++;
      }
   }
 /* test for extremum at f=0.5  */
 j = Max_Grid_Indx;
 if( ( (Error[j]>0.0) && 
       (Error[j]>Error[j-1]) && 
       (fabs(Error[j])>=Abs_Delta) ) ||
     ( (Error[j]<0.0) && 
       (Error[j]<Error[j-1]) && 
       (fabs(Error[j])>=Abs_Delta) ) )
   { 
    Ext_Freq[k]=Max_Grid_Indx;
    k++;
   }
 /*----------------------------------------------------*/
 /*  find and remove superfluous extremal frequencies  */
 if( k>Num_Approx_Funcs+1)
   {
    extras = k - (Num_Approx_Funcs+1);
    for(i=1; i<=extras; i++)
      {
       minVal = fabs(Error[Ext_Freq[0]]);
       indexOfSmallest = 0;
       for(j=1; j< k; j++)
         {
          if(fabs(Error[Ext_Freq[j]]) >= minVal) continue;
            minVal = fabs(Error[Ext_Freq[j]]);
            indexOfSmallest = j;
         }
       k--;
       for(j=indexOfSmallest; j<k; j++) Ext_Freq[j] = Ext_Freq[j+1];
      }
   }
 return;
} 
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

int RemezAlgorithm::RemezStop2()
{
 double maxVal, minVal, qq;
 int j, result;

 result = 0;
 maxVal = fabs(Error[Ext_Freq[0]]);
 minVal = fabs(Error[Ext_Freq[0]]);

 for( j=1; j<= Num_Approx_Funcs; j++)
   {
    if(fabs(Error[Ext_Freq[j]]) < minVal) minVal = fabs(Error[Ext_Freq[j]]);
    if(fabs(Error[Ext_Freq[j]]) > maxVal) maxVal = fabs(Error[Ext_Freq[j]]);
   }                                                                
 qq = (maxVal - minVal)/maxVal;
 if(qq<0.01) result = 1;
 return(result);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

int RemezAlgorithm::RemezStop( void)
{
 //static int Old_Ext_Freq[50];
 int j, result;
 
 result = 1;
 
 for(j=0; j<=Num_Approx_Funcs; j++)
   {
    if(Ext_Freq[j] != Old_Ext_Freq[j]) result = 0;
    Old_Ext_Freq[j] = Ext_Freq[j];
   }
 return(result);
}   
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

double RemezAlgorithm::WeightFunction(double freq)
{
 double result;
 
 result = 1.0;
 if(freq <= PB_Edge_Freq) result = 1.0/Ripple_Ratio;
 return(result);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

double RemezAlgorithm::DesiredResponse(double freq)
{
 double result;
 
 result = 0.0;
 if(freq <= PB_Edge_Freq) result = 1.0;
 return(result);
}

