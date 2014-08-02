//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  File = impresp.cpp
//
//

#include <math.h>
#include <stdlib.h>
#include "impresp.h"
#include "complex.h"

#ifdef _DEBUG
extern std::ofstream DebugFile;
#endif

//===========================================================
//  constructors

ImpulseResponse::ImpulseResponse( FilterTransFunc *trans_func,
                                  int num_resp_pts,
                                  double delta_time)
{ 
  complex k_sub_r_denom, k_sub_r_numer, s_sub_r;
  int r, n;

  Delta_Time = delta_time;
  Num_Resp_Pts = num_resp_pts;
  Trans_Func = trans_func;

  H_Sub_Zero = Trans_Func->GetHSubZero();
  Num_Poles = Trans_Func->GetNumPoles();
  Num_Zeros = Trans_Func->GetNumZeros();

  #ifdef _DEBUG
  DebugFile << "Num_Poles = " << Num_Poles << std::endl;
  DebugFile << "Num_Zeros = " << Num_Zeros << std::endl;
  #endif

  R_Max = (Num_Poles+1)>>1;
  Order_Is_Odd = Num_Poles%2;

  #ifdef _DEBUG
  DebugFile << "R_Max = " << R_Max << std::endl;
  #endif

  K_Sub_R = new complex[Num_Poles + 1];
  Sigma = new double[Num_Poles + 1];
  Omega = new double[Num_Poles + 1];

  //-----------------------------------
  //  compute Kr coefficients

  for(r=1; r<=Num_Poles; r++)
    {
    #ifdef _DEBUG
    DebugFile << "r = " << r << std::endl;
    #endif
    k_sub_r_denom = complex(1.0, 0.0);
    k_sub_r_numer = complex(1.0, 0.0);
    s_sub_r = Trans_Func->GetPole(r);
    #ifdef _DEBUG
    DebugFile << "s_sub_r = " << s_sub_r << std::endl;
    #endif
    Sigma[r] = real(s_sub_r);
    Omega[r] = imag(s_sub_r);
    for(n=1; n<=Num_Poles; n++)
      {
      if(n==r) continue;
        #ifdef _DEBUG
        DebugFile << "pole[" << n << "] = "
                  << (Trans_Func->GetPole(n)) << std::endl;
        DebugFile << "difference term = "
                  << (s_sub_r - (Trans_Func->GetPole(n)))
                  << std::endl;
        #endif
        k_sub_r_denom *= (s_sub_r - (Trans_Func->GetPole(n)));
        #ifdef _DEBUG
        DebugFile << "k_sub_r_denom = " << k_sub_r_denom
                  << std::endl;
        #endif
      }
    for(n=1; n<=Num_Zeros; n++)
      {
      #ifdef _DEBUG
      DebugFile << "zero[" << n << "] = "
                << (Trans_Func->GetZero(n)) << std::endl;
      DebugFile << "difference term = "
                << (s_sub_r - (Trans_Func->GetZero(n)))
                << std::endl;
      #endif
      k_sub_r_numer *= (s_sub_r - (Trans_Func->GetZero(n)));
      #ifdef _DEBUG
      DebugFile << "k_sub_r_numer = " << k_sub_r_numer
                << std::endl;
      #endif
      }
    K_Sub_R[r] = k_sub_r_numer/k_sub_r_denom;
    #ifdef _DEBUG
    DebugFile << "K_Sub_R[" << r << "] = "
              << K_Sub_R[r] << std::endl;
    #endif
    }

  return;
};      

//=========================================================
void ImpulseResponse::GenerateResponse( void )
  {
  int resp_indx;
  double h_of_t, time, delta_t;

  Response_File = new ofstream("imp_anal.txt", ios::out);

  //-----------------------------------------------
  // compute samples of impulse response

  delta_t = Delta_Time;

  for(resp_indx=0; resp_indx<Num_Resp_Pts; resp_indx++)
    {
    time = delta_t * resp_indx;
    h_of_t = ComputeSample(time);
    (*Response_File) << time << ",  " << h_of_t << std::endl;
    }
  Response_File->close();
  return;
}

//=========================================================
double ImpulseResponse::ComputeSample( double time )
  {
  int r;
  double cos_part, sin_part;
  double h_of_t;

  h_of_t = 0.0;

  for(r=1; r<=(Num_Poles>>1); r++)
    {
    cos_part = 2 * real(K_Sub_R[r]) * 
               exp(Sigma[r] * time) *
               cos(Omega[r] * time);
    sin_part = 2 * imag(K_Sub_R[r]) * 
               exp(Sigma[r] * time) *
               sin(Omega[r] * time);
    h_of_t += (cos_part - sin_part);
    }
  //---------------------------------------------
  //  add the real exponential component
  //  present in odd-order responses

  if(Order_Is_Odd == 1)
    {
    h_of_t += ( real(K_Sub_R[R_Max]) * 
                  exp(Sigma[R_Max] * time) );
    }
  h_of_t *= H_Sub_Zero;

  return(h_of_t);
}

