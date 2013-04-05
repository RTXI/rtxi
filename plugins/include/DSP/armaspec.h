//
//  File = arma_est.h
//

#ifndef _ARMASPEC_H_
#define _ARMASPEC_H_

#include "complex.h"
#include "typedefs.h"
#include "thy_spec.h"

template< class T>
class ArmaSpectrum
{
public:
  ArmaSpectrum( int ar_order,
                T* ar_coeff,
                int ma_order,
                T* ma_coeff,
                double samp_invl,
                double drv_variance);

  ~ArmaSpectrum(void);
  void DumpSpectrum( char* out_file_nam,
                     TheoreticalSpectrum *ref_spectrum,
                     logical db_plot_enab);
private:
  int Ar_Order;
  int Num_Pts;
  double Max_Freq;
  double Freq_Delt;
  double *Spec_Buf;
  double Samp_Intvl;
};

#endif
