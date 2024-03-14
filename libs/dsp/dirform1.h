//
//  File = dirform1.h
//

#ifndef _DIRFORM1_H_
#define _DIRFORM1_H_

#include <cstddef>
#include <cstdint>
#include <vector>
#include "filt_imp.h"

/*!
  * Direct Form Filter Implementation
  *
  * \sa FilterImplementation
  */
class DirectFormFir : public FilterImplementation
{
public:
  DirectFormFir(int num_taps,
                double* coeff,
                bool quan_enab,
                int64_t coeff_quan_factor,
                int64_t input_quan_factor);
  double ProcessSample(double input_val) override;
  int64_t ProcessSample(int64_t input_val) override;
  int GetNumTaps() override;

protected:
  std::vector<double>& getUnquanCoeff() { return Unquan_Coeff; }

private:
  size_t Num_Taps;
  std::vector<int64_t> Quan_In_Buf;
  std::vector<int64_t> Quan_Coeff;
  std::vector<double> Unquan_In_Buf;
  std::vector<double> Unquan_Coeff;
  bool Quan_Enab;
  int Write_Indx;
  int64_t Input_Quan_Factor;
  double Output_Quan_Factor;
  int64_t int64_t_Out_Quan_Factor;
};

#endif
