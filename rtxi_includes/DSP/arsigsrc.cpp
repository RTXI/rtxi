//
//  File = arsigsrc.cpp
//
#include <iostream> 
#include <iostream>
#include <fstream>
#include "pause.h"
#include "complex.h"
#include "polefunc.h"
#include "bilinear.h"
#include "unq_iir.h"
#include "arsigsrc.h"
#include "gausrand.h"
   

#ifdef _DEBUG
  extern std::ofstream DebugFile;
#endif

void ArSignalSource( istream& uin, ostream& uout, 
                     double *ret_samp_intvl, 
                     int seq_len, 
                     complex *signal)
{
  int i;
  int ar_order;
  int sim_cyc_per_samp;
  double delta_t, out_samp, samp_intvl;
  double *denom_coeff=NULL;
  double single_numer_coeff[1];
  complex *cp_roots;
  int ii, spec_meth;
  FilterTransFunc* anlg_filt_func;
  IirFilterDesign *dig_filt_dsgn;
  FilterImplementation *filt_implem;
  CmplxPolynomial *char_poly;
  long noise_seed=1117753;
  

  //- - - - - - - - - - - - - - - - - - - - - - - - - -
  //  determine parameters to create a test signal

  ofstream inp_seq_file("inp_seq.txt", ios::out);

  uout << "time increment for simulated analog waveform?" << std::endl;
  uin >> delta_t;
  uout << "decim rate for discrete time seq?" << std::endl;
  uin >> sim_cyc_per_samp;
  samp_intvl = sim_cyc_per_samp * delta_t;
  *ret_samp_intvl = samp_intvl;

  uout << "How is AR process to be specified?\n"
       << "  1 = AR coefficients\n"
       << "  2 = pole locations" << std::endl;
  uin >> spec_meth;

  int cp_order;
  double d_val;
  switch (spec_meth){
  case 1:   // AR coefficients
    uout << "order of AR process?" << std::endl;
    uin >> cp_order;
    ar_order = cp_order;
    denom_coeff = new double[cp_order+1];
    denom_coeff[0] = 0.0;
    for(ii=1; ii<=cp_order; ii++)
      {
      uout << "enter a[" << ii << "] " << flush;
      uin >> d_val;
      denom_coeff[ii] = d_val;
      }
    break;
  case 2:   // pole locations
    anlg_filt_func = new AllPoleTransFunc(uin, uout);
    anlg_filt_func->LowpassDenorm(1.0);
    ar_order = anlg_filt_func->GetNumPoles();

    anlg_filt_func->FrequencyPrewarp( samp_intvl );
    dig_filt_dsgn = BilinearTransf( anlg_filt_func,
                                    samp_intvl);
    dig_filt_dsgn->DumpCoefficients( &DebugFile);
    delete anlg_filt_func;

    //------------------------------------------------------
    //  optional chance for user to modify IIR coefficients

    cp_order = dig_filt_dsgn->GetNumDenomCoeffs();
    denom_coeff = dig_filt_dsgn->GetDenomCoefficients();

    int mod_flag;
    uout << "want chance to modify IIR coeffs?" << std::endl;
    uout << "( 0=NO, 1=YES )" << std::endl;
    uin >> mod_flag;
    if(mod_flag >0)
      {
      uout << setprecision(10);
      for(ii=1; ii<=cp_order; ii++)
        {
        d_val = denom_coeff[ii];
        uout << "denom_coeff[" << ii << "] = " 
             << d_val << std::endl;
        uout << "enter modified value " << std::endl;
        uin >> d_val;
        denom_coeff[ii] = d_val;
        }
      uout << setprecision(6) << flush;
      }
    break;
  } //end of switch(spec_meth)
  //--------------------------------------------------------
  //  Check to see if test signal generator will be stable

  complex* cp_coeff = new complex[cp_order+1];
  cp_coeff[cp_order] = complex(1.0,0.0);
  for( ii=0; ii<cp_order; ii++)
    cp_coeff[ii] = -denom_coeff[cp_order-ii];

  char_poly = new CmplxPolynomial( cp_coeff, cp_order );
  cp_roots = char_poly->GetRoots();

  for( ii=0; ii<ar_order; ii++)
    {
    if( cabs(cp_roots[ii])<1.0) continue;
      uout << "generator root " << ii << " (val=" 
           << cp_roots[ii] << ")\n is outside the unit circle" 
           << std::endl;
      char_poly->ReflectRoot(ii);
    }
  delete [] cp_coeff;
  delete char_poly;

  //--------------------------------------------------------
  //  Generate segment of test signal

  single_numer_coeff[0] = 1.0;     
  filt_implem = new UnquantDirectFormIir(
                 1,
                 cp_order,
                 single_numer_coeff,
                 denom_coeff);
  double noise_pwr;
  uout << "power of noise to be added to signal?" << std::endl;
  uin >> noise_pwr;

  double std_dev = sqrt(noise_pwr);                

  i=0;
  out_samp = filt_implem->ProcessSample(1.0/samp_intvl);
  if(std_dev >0.0) 
         out_samp += std_dev * GaussRandom(&noise_seed);
  signal[i] = out_samp;
  inp_seq_file << i*samp_intvl << ", " << out_samp << std::endl;

  for(i=1; i<seq_len; i++)
    {
    out_samp = filt_implem->ProcessSample(0.0);
    if(std_dev >0.0) 
           out_samp += std_dev * GaussRandom(&noise_seed);
    signal[i] = out_samp;
    inp_seq_file << i*samp_intvl << ", " << out_samp << std::endl;
    }
  inp_seq_file.close();
  delete filt_implem;

  return;
}  
