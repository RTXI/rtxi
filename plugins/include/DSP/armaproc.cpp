//
//  File = armaproc.cpp
//

#include <stdlib.h>
#include <fstream>
#include "armaproc.h"
#include "gausrand.h"
#include "yulewalk.h"
#include "mod_yuwa.h"

//============================================
//  default constructor

template<class T>
ArmaProcess<T>::ArmaProcess( void )
{
Ar_Order = 0;
A_Coeffs = NULL;
B_Coeffs = NULL;
Old_Output = NULL;
}
//====================================
//  destructor

template<class T>
ArmaProcess<T>::~ArmaProcess( void )
{
delete A_Coeffs;
delete B_Coeffs;
delete Old_Output;
}

//============================================
//  Function to dump ARMA parameters to output
//  stream indicated by uout

template <class T>
void ArmaProcess<T>::DumpParameters(ostream& uout)
  {
  int indx;
  uout << "Drv_Noise_Var = " << Drv_Noise_Var << std::endl;

  if(Ar_Order > 0)
    {
    for(indx=0; indx<=Ar_Order; indx++)
                  uout << "a[" << indx << "] = " 
                       << A_Coeffs[indx] << std::endl;
    }

  if(Ma_Order > 0)
    {
    for(indx=0; indx<=Ma_Order; indx++)
                  uout << "b[" << indx << "] = " 
                       << B_Coeffs[indx] << std::endl;
    }

  return;
  }

//=================================================
//  function to generate an output sequence of
//  length seq_len.  If noise_seed_init is nonzero,
//  value supplied will be used as the new seed
//  for the noise generator.  Otherwise, the stored
//  value will be used.

template <class T>
T* ArmaProcess<T>::OutputSequence( long noise_seed_init,
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
    for(i=Ma_Order; i>0; i--)
      {
      Old_Input[i] = Old_Input[i-1];
      }
    out_samp = std_dev * GaussRandom(&noise_seed);
    Old_Input[0] = out_samp;
    for(i=1; i<=Ma_Order; i++)
      {
      out_samp += B_Coeffs[i]*Old_Input[i];
      }
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
    } // end of loop over samp_indx

  Noise_Seed = noise_seed;
  return(out_seq);
}
template <class T>
void ArmaProcess<T>::GetParameters( int* ar_order,
                                    T* a_coeff,
                                    int* ma_order,
                                    T* b_coeff)
  {
  int indx;
  *ar_order = Ar_Order;
  for(indx=0; indx<=Ar_Order; indx++)
    {
    a_coeff[indx] = A_Coeffs[indx];
    }
  *ma_order = Ma_Order;
  for(indx=1; indx<=Ma_Order; indx++)
    {
    b_coeff[indx] = B_Coeffs[indx];
    }
  return;
  }
//---------------------------------------------------
template<class T>
double ArmaProcess<T>::GetDrivingVariance( void )
{
  return(Drv_Noise_Var);
}
template ArmaProcess<double>;
template ArmaProcess<complex>;

