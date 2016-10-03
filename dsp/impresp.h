//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  File = impresp.h
//
//

#ifndef _IMPRESP_H_
#define _IMPRESP_H_

#include <fstream>
#include "poly.h"
#include "typedefs.h"
#include "filtfunc.h"

class ImpulseResponse
{ 
 public: 
   
   ImpulseResponse( FilterTransFunc* trans_func,
                    int num_resp_pts,
                    double delta_t);
   void GenerateResponse(void);
   double ComputeSample(double time);
   
 private:
 
   FilterTransFunc *Trans_Func;
   double Delta_Time;
   int Num_Resp_Pts;
   std::ofstream *Response_File;
   double H_Sub_Zero;
   complex* K_Sub_R;
   double *Sigma, *Omega;
   int Num_Poles, Num_Zeros;
   int Order_Is_Odd;
   int R_Max;
};

#endif
