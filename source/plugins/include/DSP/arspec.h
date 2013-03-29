//
//  File = ar_est.h
//

#ifndef _ARSPEC_H_
#define _ARSPEC_H_

#include "complex.h"
#include "typedefs.h"
#include "thy_spec.h"

template< class T>
class ArSpectrum
{
public:
  ArSpectrum( int ar_order,
              T* ar_coeff,
              double samp_invl,
              double drv_variance);

  ~ArSpectrum(void);
  void DumpSpectrum( char* out_file_name,
                     TheoreticalSpectrum *ref_spectrum,
                     logical db_plot_enab);
  void DumpSpectrum( char* out_file_name,
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
