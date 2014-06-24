//
//  File = ma_proc.cpp
//

#include <stdlib.h>
#include <fstream>
#include "ma_proc.h"
#include "gausrand.h"
#include "yulewalk.h"
#include "sig_type.h"

#ifdef _DEBUG
  extern std::ofstream DebugFile;
#endif

//=============================================
//  default constructor 

template < class T >
MaProcess<T>::MaProcess( void )
{
  B_Coeffs = NULL;
  Old_Input = NULL;
}
//=============================================
//  destructor 

template < class T >
MaProcess<T>::~MaProcess( void )
{
  delete B_Coeffs;
  delete Old_Input;
}

//============================================
//  Function to dump MA parameters to output
//  stream indicated by uout

template < class T >
void MaProcess<T>::DumpParameters(ostream& uout)
  {
  uout << "Drv_Noise_Var = " << Drv_Noise_Var << std::endl;
  for(int indx=0; indx<=Ma_Order; indx++)
    {
    uout << "b[" << indx << "] = " << B_Coeffs[indx] << std::endl;
    }
  return;
  }

//=================================================
//  function to generate an output sequence of
//  length seq_len.  If noise_seed_init is nonzero,
//  value supplied will be used as the new seed
//  for the noise generator.  Otherwise, the stored
//  value will be used.

template < class T >
T* MaProcess<T>::OutputSequence( long noise_seed_init,
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

    out_seq[samp_indx] = out_samp;
    } // end of loop over samp_indx

  Noise_Seed = noise_seed;
  return(out_seq);
}
//--------------------------------
//  Explicit instantiations
template MaProcess< type_of_sig_vals_T >;
