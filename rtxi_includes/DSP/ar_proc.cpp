//
//  File = ar_proc.cpp
//

#include <stdlib.h>
#include <fstream>
#include "ar_proc.h"
#include "gausrand.h"
#include "sig_type.h"

#ifdef _DEBUG
  extern std::ofstream DebugFile;
#endif

//===============================================
//  ArProcess 

template<class T>
ArProcess<T>::ArProcess( void )
{
A_Coeffs = NULL;
Old_Output = NULL;
Sum_Wgn_Samps = 0.0;
Sum_Wgn_Sqrs = 0.0;
Sum_Samps = 0.0;
Sum_Squares = 0.0;
Num_Samps = 0;
}
//==================================================
//  Destructor

template<class T>
ArProcess<T>::~ArProcess(void)
{
delete[] A_Coeffs;
delete[] Old_Output;
}
//==================================================
//  Function to dump AR parameters to output
//  stream indicated by uout

template<class T>
void ArProcess<T>::DumpParameters( ostream& uout)
  {
  uout << "Drv_Noise_Var = " << Drv_Noise_Var << std::endl;
  for(int indx=0; indx<=Ar_Order; indx++)
    {
    uout << "a[" << indx << "] = " << A_Coeffs[indx] << std::endl;
    }
  return;
  }
//==================================================
//  Function to copy AR parameters to array
//  pointed to by a_coeff

template<class T>
void ArProcess<T>::GetParameters( int* ar_order,
                                  T* a_coeff)
  {
  *ar_order = Ar_Order;
  for(int indx=0; indx<=Ar_Order; indx++)
    {
    a_coeff[indx] = A_Coeffs[indx];
    }
  return;
  }
//---------------------------------------------------
//  function to generate an output sequence of
//  length seq_len.  If noise_seed_init is nonzero,
//  value supplied will be used as the new seed
//  for the noise generator.  Otherwise, the stored
//  value will be used.

template<class T>
T* ArProcess<T>::OutputSequence( long noise_seed_init,
                                 int seq_len )
{
  int samp_indx, i;
  T *out_seq, out_samp;
  long noise_seed;
  double std_dev;

  std_dev = sqrt(Drv_Noise_Var);
  out_seq = new T[seq_len];

  if(noise_seed_init == 0)
    noise_seed = Noise_Seed;
  else
    noise_seed = noise_seed_init;

  for(samp_indx=0; samp_indx<seq_len; samp_indx++)
    {
    out_samp = std_dev * GaussRandom(&noise_seed);
    for(i=0; i<Ar_Order; i++)
      {
      out_samp -= A_Coeffs[i+1]*Old_Output[i];
      }

    for(i=Ar_Order-1; i>0; i--)
      {
      Old_Output[i] = Old_Output[i-1];
      }
    Old_Output[0] = out_samp;
    out_seq[samp_indx] = out_samp;
    #ifdef _DEBUG
      DebugFile << samp_indx << ", " << out_samp << std::endl;
    #endif
    } // end of loop over samp_indx

  Noise_Seed = noise_seed;
  return(out_seq);
}
//---------------------------------------------------

template<class T>
T ArProcess<T>::NextSample( long noise_seed_init )
{
  Sum_Wgn_Samps = 0.0;
  Sum_Wgn_Sqrs = 0.0;
  Sum_Samps = 0.0;
  Sum_Squares = 0.0;
  Num_Samps = 0;
  if(noise_seed_init != 0)
    Noise_Seed = noise_seed_init;
  return(NextSample());
}

//---------------------------------------------------
template<class T>
T ArProcess<T>::GetMean( void )
{
  return(Sum_Samps/double(Num_Samps));
}
//---------------------------------------------------
template<class T>
double ArProcess<T>::GetDrivingVariance( void )
{
  return(Drv_Noise_Var);
}
//---------------------------------------------------
template<class T>
T ArProcess<T>::GetVariance( void )
{
  T mean,var;
  mean = Sum_Samps/double(Num_Samps);
  var = (Sum_Squares/double(Num_Samps)) - (mean*mean);
  #ifdef _DEBUG
    DebugFile << "mean = " << mean << std::endl;
    DebugFile << "variance = " << var << std::endl;
  #endif
  return(var);
}
//---------------------------------------------------

template<class T>
T ArProcess<T>::NextSample( void )
{
  int i;
  T out_samp;
  double std_dev;

  std_dev = sqrt(Drv_Noise_Var);

  out_samp = std_dev * GaussRandom(&Noise_Seed);
  Sum_Wgn_Samps += out_samp;
  Sum_Wgn_Sqrs += out_samp*out_samp;
  for(i=0; i<Ar_Order; i++)
    {
    out_samp -= A_Coeffs[i+1]*Old_Output[i];
    }

  for(i=Ar_Order-1; i>0; i--)
    {
    Old_Output[i] = Old_Output[i-1];
    }
  Old_Output[0] = out_samp;
  Sum_Samps += out_samp;
  Sum_Squares += (out_samp*out_samp);
  Num_Samps++;

  return(out_samp);
}
template ArProcess<type_of_sig_vals_T>;


