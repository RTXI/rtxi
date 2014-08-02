//
//  File = armaspec.cpp
//

#include <stdlib.h>
#include <fstream>
#include "armaspec.h"
#include "misdefs.h"

//==============================================
//  constructor

template < class T >
ArmaSpectrum<T>::ArmaSpectrum( int ar_order,
                               T* ar_coeff,
                               int ma_order,
                               T* ma_coeff,
                               double samp_intvl,
                               double drv_var)
{
  double numer, denom, two_pi_f;
  double psd_val;
  double a_func_real, a_func_imag;
  double b_func_real, b_func_imag;
  int f_idx, cof_idx;

  Samp_Intvl = samp_intvl;
  std::cout << "Now preparing ARMA spectrum estimate" << std::endl;
  std::cout << "How many points in spectrum plot?" << std::endl;
  std::cin >> Num_Pts;
  Spec_Buf = new double[Num_Pts];
  std::cout << "Maximum frequency?" << std::endl;
  std::cin >> Max_Freq;
  Freq_Delt = Max_Freq/(Num_Pts-1);
  for(f_idx=0; f_idx<Num_Pts; f_idx++)
    {
    a_func_real = 0.0;
    a_func_imag = 0.0;
    b_func_real = 0.0;
    b_func_imag = 0.0;
    two_pi_f = TWO_PI * f_idx * Freq_Delt;
    for(cof_idx=0; cof_idx<=ar_order; cof_idx++)
      {
      a_func_real += (ar_coeff[cof_idx]*cos(cof_idx*two_pi_f));
      a_func_imag -= (ar_coeff[cof_idx]*sin(cof_idx*two_pi_f));
      }
    denom = a_func_real*a_func_real + a_func_imag*a_func_imag;
    for(cof_idx=1; cof_idx<=ma_order; cof_idx++)
      {
      b_func_real += (ma_coeff[cof_idx]*cos(cof_idx*two_pi_f));
      b_func_imag -= (ma_coeff[cof_idx]*sin(cof_idx*two_pi_f));
      }
    numer = b_func_real*b_func_real + b_func_imag*b_func_imag;
    psd_val = samp_intvl*drv_var*numer/denom;
    Spec_Buf[f_idx] = psd_val;
    }
  return;
}

template < class T >
ArmaSpectrum<T>::~ArmaSpectrum(void){ };

template < class T >
void ArmaSpectrum<T>::DumpSpectrum( char* out_file_nam,
                                    TheoreticalSpectrum *ref_spectrum,
                                    logical db_plot_enab )
{
  int i;
  double freq, ref_value, ref_offset, vert_offset;
  ofstream out_file(out_file_nam, ios::out);

  vert_offset = 10.0 * log10(Spec_Buf[0]);
  ref_value = 1.0;
  ref_offset = 0.0;
  if(ref_spectrum != NULL) ref_offset = 
                  10.0 * log10(ref_spectrum->GetPsdValue( 0.0 ));
  for(i=0; i<Num_Pts; i++)
    {
    freq = i*Freq_Delt;
    if(ref_spectrum != NULL) 
         ref_value = ref_spectrum->GetPsdValue( freq/Samp_Intvl);
    if( db_plot_enab) {
      out_file << freq << ", " 
               << ((10.0 * log10(Spec_Buf[i]))-vert_offset) 
               << ", "
               << ((10.0* log10(ref_value))-ref_offset) << std::endl;
      }
    else {
      out_file << freq << ", " << (Spec_Buf[i])
               << ", " << ref_value << std::endl;
      }
    }
  out_file.close();
}

//template ArSpectrum<complex>;
template ArmaSpectrum<double>;

