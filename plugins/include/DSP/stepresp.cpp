//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  File = stepresp.cpp
//
//

#include <math.h>
#include <stdlib.h>
#include "stepresp.h"
#include "complex.h"

#ifdef _DEBUG
extern std::ofstream DebugFile;
#endif

//===========================================================
//  constructors

StepResponse::StepResponse( FilterTransFunc *trans_func,
                            int num_resp_pts,
                            double delta_time)
{ 
  Delta_Time = delta_time;
  Num_Resp_Pts = num_resp_pts;
  Imp_Resp = new ImpulseResponse( trans_func,
                                  num_resp_pts,
                                  delta_time);

  return;
};      

//=========================================================
void StepResponse::GenerateResponse( void )
  {
  int resp_indx;
  double h_of_t, time, delta_t;
  double u_of_t;

  Response_File = new ofstream("stp_anal.txt", ios::out);

  //-----------------------------------------------
  // compute samples of impulse response

  delta_t = Delta_Time;

  for(resp_indx=0; resp_indx<Num_Resp_Pts; resp_indx++)
    {
    time = delta_t * resp_indx;
    h_of_t = Imp_Resp->ComputeSample(time);
    u_of_t += (delta_t * h_of_t);
    (*Response_File) << time << ",  " << u_of_t << std::endl;
    }
  Response_File->close();
  return;
}

