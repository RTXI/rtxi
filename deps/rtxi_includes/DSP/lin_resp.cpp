//
//  File = lin_resp.cpp
//

#include <math.h>
#include <stdlib.h>
#include "lin_resp.h"
#include "typedefs.h"
#include "misdefs.h"
#ifdef _DEBUG
extern std::ofstream DebugFile;
#endif


//==================================================================
// constructor with interactive setting of configuration parameters
//------------------------------------------------------------------
LinearPhaseFirResponse::LinearPhaseFirResponse( LinearPhaseFirDesign *filter_design,
                                                istream& uin,
                                                ostream& uout )
                       :FirFilterResponse( filter_design,
                                           uin,
                                           uout)
{
} 

//=========================================================
//  method to compute magnitude response
//
//  note: this function redefines the virtual function
//        ComputeMagResp that is defined in the base 
//        class FirFilterResponse
//---------------------------------------------------------

void LinearPhaseFirResponse::ComputeMagResp( void )
{
int index, resp_indx, tap_indx;
double lambda, work;

double* coeff = Filter_Design->GetCoefficients();
int fir_type = ((LinearPhaseFirDesign*)Filter_Design)->GetFirType();


for( resp_indx=0; resp_indx<Num_Resp_Pts; resp_indx++)
  {
  lambda = resp_indx * PI / (double) Num_Resp_Pts;
   
  switch (fir_type) {
  
    case 1:     /* symmetric and odd */
      work = coeff[(Num_Taps-1)/2];
      for( tap_indx=1; tap_indx<=((Num_Taps-1)/2); tap_indx++)
        {
        index = (Num_Taps-1)/2 - tap_indx;
        work = work + 2.0 * coeff[index] * cos(tap_indx*lambda);
        }
      break;
      
    case 2:     /* symmetric and even */
      work = 0.0;
      for( tap_indx=1; tap_indx<=(Num_Taps/2); tap_indx++) 
        {
        index = Num_Taps/2-tap_indx;
        work = work + 2.0 * coeff[index] * cos((tap_indx-0.5)*lambda);
        }
      break;
      
    case 3:     /* antisymmetric and odd */
      work = 0.0;
      for( tap_indx=1; tap_indx<=((Num_Taps-1)/2); tap_indx++)
        {
        index = (Num_Taps-1)/2 - tap_indx;
        work = work + 2.0 * coeff[index] * sin(tap_indx*lambda);
        }
      break;
      
    case 4:     /* symmetric and even */
      work = 0.0;
      for( tap_indx=1; tap_indx<=(Num_Taps/2); tap_indx++)
        {
        index = Num_Taps/2-tap_indx;
        work = work + 2.0 * coeff[index] * sin((tap_indx-0.5)*lambda);
        }
      break;
    }
      
  if(Db_Scale_Enabled)
    {Mag_Resp[resp_indx] = 20.0 * log10(fabs(work));}
  else
    {Mag_Resp[resp_indx] = fabs(work);}
  }
return;
}
