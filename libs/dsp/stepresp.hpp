//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  File = stepresp.h
//
//

#ifndef _STEPRESP_H_
#define _STEPRESP_H_

#include "filtfunc.h"
#include "impresp.h"
#include "poly.h"
#include "typedefs.h"
#include <fstream>

class StepResponse
{
public:
  StepResponse(FilterTransFunc* trans_func, int num_resp_pts, double delta_t);
  void GenerateResponse(void);

protected:
  ImpulseResponse* Imp_Resp;
  double Delta_Time;
  int Num_Resp_Pts;
  std::ofstream* Response_File;
};

#endif
