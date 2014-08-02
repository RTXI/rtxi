//
//  File = iirswept.h
//

#ifndef _IIRSWEPT_H_
#define _IIRSWEPT_H_

#include "filt_imp.h"

class SweptResponse
{
public:
  SweptResponse( FilterImplementation *filter_implem,
                 double sampling_interval,
                 std::istream& uin,
                 std::ostream& uout );
  ~SweptResponse();
  void SweptResponse::NormalizeResponse( void );
  void DumpMagResp( void );

private:
  int Num_Taps;
  int Num_Resp_Pts;
  int Db_Scale_Enabled;
  int Normalize_Enabled;
  double *Mag_Resp;
  //double *Mag_Resp_2;
  double *Phase_Resp;
  //double *Phase_Resp_2;
  double Max_Sweep_Freq;
  std::ofstream *Response_File;
  FilterImplementation *Filter_Implem;
  //FilterImplementation *Q_Filter_Implem;

};

#endif
