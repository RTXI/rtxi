//
//  File = swept.h
//

#ifndef _SWEPT_H_
#define _SWEPT_H_

#include "filt_imp.h"

class SweptResponse
{
public:
  SweptResponse( FilterImplementation *filter_implem,
                 double sampling_interval,
                 istream& uin,
                 ostream& uout );
  ~SweptResponse();
  void SweptResponse::NormalizeResponse( void );
  void DumpMagResp( void );

private:
  int Num_Taps;
  int Num_Resp_Pts;
  int Db_Scale_Enabled;
  int Normalize_Enabled;
  double *Mag_Resp;
  double *Phase_Resp;
  double Max_Sweep_Freq;
  ofstream *Response_File;
  FilterImplementation *Filter_Implem;

};

#endif
