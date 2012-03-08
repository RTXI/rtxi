//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  File = filtfunc.cpp
//
//

#include <math.h>
#include <iostream>
#include <stdlib.h>
#include "misdefs.h"
#include "filtfunc.h"
#include "unwrap.h"
#include "cmpxpoly.h" 

#ifdef _DEBUG
extern std::ofstream DebugFile;
#endif

//===========================================================
//  constructor

FilterTransFunc::FilterTransFunc( void )
{ 
 Num_Denorm_Poles = 0;
 Num_Denorm_Zeros = 0;
 Degree_Of_Denom = -1;
 Degree_Of_Numer = -1;
 Filter_Is_Denormalized = FALSE;
 return;
};      


//===========================================================
//  constructor

FilterTransFunc::FilterTransFunc( int order )
{ 
 Filter_Order = order;
 Num_Denorm_Poles = 0;
 Num_Denorm_Zeros = 0;
 Degree_Of_Denom = -1;
 Degree_Of_Numer = -1;
 Filter_Is_Denormalized = FALSE;
 return;
};      


//==========================================================
//
complex* FilterTransFunc::GetPrototypePoles( int *num_poles )   
{ 
 *num_poles = Num_Prototype_Poles;
 return(Prototype_Pole_Locs);
};                           

//==========================================================
//
complex* FilterTransFunc::GetPoles( int *num_poles )   
{ 
 if(Filter_Is_Denormalized)
  {
   *num_poles = Num_Denorm_Poles;
   return(Denorm_Pole_Locs);
  }
 else
  {
   *num_poles = Num_Prototype_Poles;
   return(Prototype_Pole_Locs);   
  }
};                           
//==========================================================
//
complex FilterTransFunc::GetPole( int pole_indx )   
{ 
if(Filter_Is_Denormalized)
  {
  if(pole_indx > Num_Denorm_Poles)  
    {return(complex(0.0,0.0));}
  else
    {return(Denorm_Pole_Locs[pole_indx]);}
  }
else
  {
  if(pole_indx > Num_Prototype_Poles)
    {return(complex(0.0,0.0));}
  else
    {return(Prototype_Pole_Locs[pole_indx]);}
  }
};                           
//==========================================================
//
complex FilterTransFunc::GetZero( int zero_indx )   
{ 
if(Filter_Is_Denormalized)
  {
  if(zero_indx > Num_Denorm_Zeros)  
    {return(complex(0.0,0.0));}
  else
    {return(Denorm_Zero_Locs[zero_indx]);}
  }
else
  {
  if(zero_indx > Num_Prototype_Zeros)
    {return(complex(0.0,0.0));}
  else
    {return(Prototype_Zero_Locs[zero_indx]);}
  }
};                           

//========================================================
void FilterTransFunc::FrequencyPrewarp( 
                              double sampling_interval)
{
 int n;
 double freq_scale, warped_analog_cutoff;
 double desired_digital_cutoff;

 desired_digital_cutoff = Denorm_Cutoff_Freq_Rad;
 warped_analog_cutoff = (2.0/sampling_interval) * 
     tan(desired_digital_cutoff * sampling_interval / 2.0);
 freq_scale = warped_analog_cutoff/desired_digital_cutoff;

 #ifdef _DEBUG
 DebugFile << "freq_scale = " << freq_scale << std::endl;
 DebugFile << "Num_Denorm_Poles = "
           << Num_Denorm_Poles << std::endl;
 DebugFile << "Num_Denorm_Zeros = "
           << Num_Denorm_Zeros << std::endl;
 DebugFile << "in prewarp, orig H_Sub_Zero = "
           << H_Sub_Zero << std::endl;
 #endif

 for(n=1; n<=Num_Denorm_Poles; n++)
   {
    Denorm_Pole_Locs[n] *= freq_scale;
   }
 for(n=1; n<=Num_Denorm_Zeros; n++)
   {
    Denorm_Zero_Locs[n] *= freq_scale;
   }
 for(n=1; n<=(Num_Denorm_Poles-Num_Denorm_Zeros);n++)
   {
    H_Sub_Zero *= freq_scale;
   }
#ifdef _DEBUG
 DebugFile << "scaled H_Sub_Zero = "
           << H_Sub_Zero << std::endl;
#endif
 return;
}

//=========================================================
void FilterTransFunc::FilterFrequencyResponse(void)
{
 complex numer, denom;
 complex transfer_function;
 complex s_val, pole;
 double delta_freq, magnitude, phase;
 double peak_magnitude;
 double *mag_resp, *phase_resp, *group_dly;
 int i, k;

 delta_freq = 0.0125;
 peak_magnitude = -1000.0;

 Response_File = new std::ofstream("anlg_rsp.txt", std::ios::out);
 mag_resp = new double[800];
 phase_resp = new double[800];
 group_dly = new double[800];

 for( i=1; i<800; i++)
   {
    numer = complex(1.0, 0.0);
    denom = complex(1.0, 0.0);
    s_val = complex(0.0, i*delta_freq);

    for( k=1; k<=Num_Denorm_Zeros; k++)
      {
       numer *= (s_val - Denorm_Zero_Locs[k]);
      }

    for( k=1; k<=Num_Denorm_Poles; k++)
      {
       denom *= (s_val - Denorm_Pole_Locs[k]);
      }
    transfer_function = numer/denom;
    magnitude = 10.0 * log10(mag_sqrd(transfer_function));
    mag_resp[i] = magnitude;
    if(magnitude > peak_magnitude)
      {
       peak_magnitude = magnitude;
      }
    phase = 180.0 * arg(transfer_function)/PI;
    phase_resp[i] = phase;
   }
 UnwrapPhase(0, &(phase_resp[1]));
 for(i=2; i<800; i++)
  {
  UnwrapPhase(1, &(phase_resp[i]));
  }
 group_dly[1] = PI * (phase_resp[1] - phase_resp[2])
                / (180.0 * delta_freq);
 for(i=2; i<800; i++)
  {
  group_dly[i] = PI * (phase_resp[i-1] - phase_resp[i])
                 / (180.0 * delta_freq);
  }
 for(i=1; i<800; i++)
   {
    (*Response_File) << i*delta_freq << ",  "
                     << (mag_resp[i]-peak_magnitude)
                     << ",  " << phase_resp[i] 
                     << ",  " << group_dly[i] << std::endl;
   }
 Response_File->close();
 delete []phase_resp;
 delete []mag_resp;
 return;
}

//==========================================================
int FilterTransFunc::GetNumPoles(void)   
{
 if(Filter_Is_Denormalized)
  {return(Num_Denorm_Poles);}
 else 
  {return(Num_Prototype_Poles);}
};                           

//==========================================================
int FilterTransFunc::GetNumZeros(void)   
{ 
 if(Filter_Is_Denormalized)
  {return(Num_Denorm_Zeros);}
 else 
  {return(Num_Prototype_Zeros);}
};                           

//===============================================================
//
complex* FilterTransFunc::GetPrototypeZeros( int *num_zeros )
{
 *num_zeros = Num_Prototype_Zeros;
 return( Prototype_Zero_Locs );
}                              

//===============================================================
//
complex* FilterTransFunc::GetZeros( int *num_zeros )
{
 if(Filter_Is_Denormalized)
  {
  *num_zeros = Num_Denorm_Zeros;
  return( Denorm_Zero_Locs );
  }
 else
  {
  *num_zeros = Num_Prototype_Zeros;
  return( Prototype_Zero_Locs );
  }
}                              


//==================================================================
//

float FilterTransFunc::GetHSubZero( void )
{
 return( (float)H_Sub_Zero);
}


//===============================================================
//

void FilterTransFunc::DumpBiquads( std::ofstream* output_stream)
{
 (*output_stream) << "\nBiquad Coefficients\n" << std::endl;
 
 for(int i=1; i<=Num_Biquad_Sects; i++)
   {
    (*output_stream) << i << ") a = " << A_Biquad_Coef[i]
                        << "    b = " << B_Biquad_Coef[i]
                        << "    c = " << C_Biquad_Coef[i]
                        << std::endl;
   }                            
 return;
}
//=======================================================
//
void FilterTransFunc::LowpassDenorm(double cutoff_freq)
{
 int j;
 Filter_Is_Denormalized = TRUE;
 Num_Denorm_Poles = Num_Prototype_Poles;
 Num_Denorm_Zeros = Num_Prototype_Zeros;
 Denorm_Pole_Locs = new complex[Num_Denorm_Poles+1];
 Denorm_Zero_Locs = new complex[Num_Denorm_Zeros+1];
 Denorm_Cutoff_Freq_Rad = cutoff_freq;

 for(j=1; j<=Num_Denorm_Poles; j++)
   {
    Denorm_Pole_Locs[j] = Prototype_Pole_Locs[j] * cutoff_freq;
   }
 for(j=1; j<=Num_Denorm_Zeros; j++)
   {
    Denorm_Zero_Locs[j] = Prototype_Zero_Locs[j] * cutoff_freq;
   }
 for(j=0; j<(Num_Denorm_Poles-Num_Denorm_Zeros); j++)
   {
    H_Sub_Zero *= cutoff_freq;
   }
 #ifdef _DEBUG
 DebugFile << "in LP denorm, H_Sub_Zero scaled to "
           << H_Sub_Zero << std::endl;
 #endif
 return;
}

//=========================================================
//

Polynomial FilterTransFunc::GetDenomPoly( void )
{
 //-----------------------------------------------------
 //  if denominator polynomial is not built yet,
 //  build it by multiplying together (s-p[i]) binomial
 //  factors where the p[i] are the poles of the filter
 
 if(Degree_Of_Denom <0)
   {
    CmplxPolynomial cmplx_denom_poly =
                           CmplxPolynomial( complex(1.0, 0.0),
                                            -Prototype_Pole_Locs[1] );
    for(int ii=2; ii<= Num_Prototype_Poles; ii++)
      {
       cmplx_denom_poly *= CmplxPolynomial( complex(1.0, 0.0),
                                            -Prototype_Pole_Locs[ii] );
      }                                                                
    #ifdef _DEBUG
    cmplx_denom_poly.DumpToStream(&DebugFile);
    #endif
    
    Denom_Poly = Polynomial( cmplx_denom_poly);
    
    Degree_Of_Denom = Denom_Poly.GetDegree();
    
    #ifdef _DEBUG
    DebugFile << "\nreal-valued version:" << std::endl;
    Denom_Poly.DumpToStream(&DebugFile);
    #endif
   }                                    
   
 return(Denom_Poly);
}


//================================================================
//

Polynomial FilterTransFunc::GetNumerPoly()
{
 //---------------------------------------------------
 //  if numerator polynomial is not built yet,
 //  build it by multiplying together (s-z[i]) binomial
 //  factors where the z[i] are the zeros of the filter.
 
 if(Degree_Of_Numer <0)
   {
    CmplxPolynomial cmplx_poly =
                           CmplxPolynomial( complex( 1.0, 0.0),
                                            -Prototype_Zero_Locs[1] );
    for(int ii=2; ii<= Num_Prototype_Zeros; ii++)
      {
       cmplx_poly *= CmplxPolynomial( complex(1.0, 0.0),
                                      -Prototype_Zero_Locs[ii] );
      }                                                          
    #ifdef _DEBUG
    cmplx_poly.DumpToStream(&DebugFile);
    #endif
    
    Numer_Poly = Polynomial(cmplx_poly);
    
    Degree_Of_Numer = Numer_Poly.GetDegree();
    
    #ifdef _DEBUG
    DebugFile << "\nreal-valued version:" << std::endl;
    Numer_Poly.DumpToStream(&DebugFile);
    #endif
   }                                    
 return(Numer_Poly);
}
