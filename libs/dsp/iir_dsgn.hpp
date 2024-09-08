//
//  File = iir_dsgn.h
//

#ifndef _IIR_DSGN_H_
#define _IIR_DSGN_H_

#include <cstdint>
#include <vector>

class IirFilterDesign
{
public:
  //-------------------------------------
  // constructor that allocates arrays
  // to hold coefficients
  IirFilterDesign(size_t num_numer_coeffs, size_t num_denom_coeffs);

  //-------------------------------------
  // constructor that allocates arrays of
  // length num_numer_coeffs and
  // num_denom_coeffs and then initializes
  // these arrays to values contained in
  // input arrays *numer_coeffs and
  // *denom_coeffs
  IirFilterDesign(size_t num_numer_coeffs,
                  size_t num_denom_coeffs,
                  std::vector<double> numer_coeffs,
                  std::vector<double> denom_coeffs);

  //------------------------------------------
  // allocate coefficient array *Imp_Resp_Coeff
  // after default constructor has been used
  void Initialize(size_t num_numer_coeffs, size_t num_denom_coeffs);

  //-------------------------------------------
  //  method to quantize coefficients
  void QuantizeCoefficients(int quant_factor, bool rounding_enabled);

  //-------------------------------------------
  //  method to scale coefficients
  void ScaleCoefficients(double scale_factor);

  //----------------------------------------
  // copy coefficients from input array
  // *coeff into array *Imp_Resp_Coeff
  void CopyCoefficients(std::vector<double>& numer_coeff,
                        std::vector<double>& denom_coeff) const;

  //----------------------------------
  // get pointers to coefficient arrays
  std::vector<double> GetNumerCoefficients() const;
  std::vector<double> GetDenomCoefficients() const;
  void SetDenomCoefficients(const std::vector<double>& coeffs);

  //---------------------------
  // get number of filter coefficients
  size_t GetNumNumerCoeffs();
  size_t GetNumDenomCoeffs();

  double GetSamplingInterval() const;
  void SetSamplingInterval(double sampling_interval);

private:
  std::vector<double> Numer_Coeffs;
  std::vector<double> Denom_Coeffs;
  std::vector<double> Orig_Numer_Coeffs;
  std::vector<double> Orig_Denom_Coeffs;
  double Sampling_Interval;
};

#endif
