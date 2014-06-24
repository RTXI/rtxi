//
//  File = iir_dsgn.h
//

#ifndef _IIR_DSGN_H_
#define _IIR_DSGN_H_ 

#include <fstream>
#include "typedefs.h"

class IirFilterDesign
{
public:
  
  //---------------------
  // default constructor
    
  IirFilterDesign( );
  
  //----------------------------
  // constructor that provides 
  // interactive initialization

  IirFilterDesign( std::istream& uin,
                   std::ostream& uout);
                   
  //-------------------------------------
  // constructor that allocates arrays 
  // to hold coefficients 
  
  IirFilterDesign( int num_numer_coeffs,
                   int num_denom_coeffs );
  
  //-------------------------------------
  // constructor that allocates arrays of
  // length num_numer_coeffs and 
  // num_denom_coeffs and then initializes
  // these arrays to values contained in
  // input arrays *numer_coeffs and
  // *denom_coeffs
  
  IirFilterDesign( int num_numer_coeffs,
                   int num_denom_coeffs,
                   double *numer_coeffs,
                   double *denom_coeffs);
                   
  //------------------------------------------ 
  // allocate coefficient array *Imp_Resp_Coeff
  // after default constructor has been used
                   
  void Initialize( int num_numer_coeffs,
                   int num_denom_coeffs );
  
  //-------------------------------------------
  //  method to quantize coefficients

  void QuantizeCoefficients( long quant_factor,
                             logical rounding_enabled );
 
  //-------------------------------------------
  //  method to scale coefficients

  void ScaleCoefficients( double scale_factor );
  
  //----------------------------------------
  // copy coefficients from input array
  // *coeff into array *Imp_Resp_Coeff 
  
  void CopyCoefficients( double *numer_coeff,
                         double *denom_coeff );
  
  //----------------------------------------------
  // dump coefficient set to output_stream 
  
  void DumpCoefficients( std::ofstream* output_stream);
 
  //----------------------------------
  // get pointers to coefficient arrays 
  
  double* GetNumerCoefficients(void);
  double* GetDenomCoefficients(void);
  void SetDenomCoefficients( int num_coeffs, double *coeffs);
  
  //---------------------------
  // get number of filter coefficients
  
  int GetNumNumerCoeffs(void);
  int GetNumDenomCoeffs(void);
  
  double GetSamplingInterval(void);
  void SetSamplingInterval(double sampling_interval);
  
protected:
  
  int Num_Numer_Coeffs;
  int Num_Denom_Coeffs;
  
  double* Numer_Coeffs;
  double* Denom_Coeffs;
  double* Orig_Numer_Coeffs;
  double* Orig_Denom_Coeffs;
  double Sampling_Interval;
};

#endif
