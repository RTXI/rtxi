//
//  File = fs_dsgn.cpp
//

#include <math.h>
#include <stdlib.h>
#include "misdefs.h"   
#include "fs_dsgn.h"

FreqSampFilterDesign::FreqSampFilterDesign( )
{
}

FreqSampFilterDesign::FreqSampFilterDesign( int band_config,
                                            int fir_type,
                                            int num_taps,
                                            double *imp_resp_coeff)
{
 Band_Config = band_config;
 Fir_Type = fir_type;
 Num_Taps = num_taps;
 Imp_Resp_Coeff = new double[num_taps];
 Original_Coeff = new double[num_taps];
 
 for(int n=0; n<num_taps; n++)
   {
    Imp_Resp_Coeff[n] = imp_resp_coeff[n];
    Original_Coeff[n] = imp_resp_coeff[n];
   }
 return;
}

FreqSampFilterDesign::FreqSampFilterDesign( FreqSampFilterSpec &filter_spec)
{
 Filter_Spec = new FreqSampFilterSpec( filter_spec);
 Num_Taps = Filter_Spec->Num_Taps;
 Fir_Type = Filter_Spec->Fir_Type;
 Imp_Resp_Coeff = new double[Num_Taps];
 Original_Coeff = new double[Num_Taps];
 return;
}

void FreqSampFilterDesign::ComputeCoefficients( FreqSampFilterSpec *filter_spec)
{
 
 int n,k, num_taps, status;
 double x, mid_pt, work;
 
 num_taps = filter_spec->GetNumTaps();
 status = 0;
 mid_pt = (num_taps-1.0)/2.0;
 switch (filter_spec->GetFirType()) 
   {
    //---------------------------------------
    case 1:     // even symmetry, odd length
      if(num_taps%2) 
        {
         for(n=0; n<num_taps; n++)
           {
            work = filter_spec->GetMagRespSamp(0);
            x = TWO_PI * (n-mid_pt)/num_taps;
            
            for(k=1; k<=(num_taps-1)/2; k++)
              {
               work += (2.0*cos(x*k)*(filter_spec->GetMagRespSamp(k)));
              }
            Imp_Resp_Coeff[n] = work/num_taps;
           }
        }
      else 
        {
         std::cout << "FATAL ERROR -- type 1 FIR cannot have even length"
              << std::endl;
         exit(-1);
        }
      break;
    //---------------------------------------
    case 2:     // even symmetry, even length
      if(num_taps%2)
        {
         std::cout << "FATAL ERROR -- type 2 FIR cannot have odd length"
              << std::endl;
         exit(-1);
        }
      else
        {
         for(n=0; n<num_taps; n++)
           {
            work = filter_spec->GetMagRespSamp(0);
            x = TWO_PI * (n-mid_pt)/num_taps;
            
            for(k=1; k<=(num_taps/2-1); k++)
              {
               work += (2.0*cos(x*k)*(filter_spec->GetMagRespSamp(k)));
              }
            Imp_Resp_Coeff[n] = work/num_taps;
           }
        }
      break;
    //---------------------------------------
    case 3:     // odd symmetry, odd length
      if(num_taps%2)
        {
         for(n=0; n<num_taps; n++)
           {
            work = 0;
            x = TWO_PI * (mid_pt-n)/num_taps;
            for(k=1; k<=(num_taps-1)/2; k++)
              {
               work += (2.0*sin(x*k)*(filter_spec->GetMagRespSamp(k)));
              }
            Imp_Resp_Coeff[n] = work/num_taps;
           }
        }
      else
        {
         std::cout << "FATAL ERROR -- type 3 FIR cannot have even length"
              << std::endl;
         exit(-1);
        }
      break;
    //---------------------------------------
    case 4:     // odd symmetry, even length
      if(num_taps%2)
        {
         std::cout << "FATAL ERROR -- type 4 FIR cannot have odd length"
              << std::endl;
         exit(-1);
        }
      else 
        {
         for(n=0; n<num_taps; n++)
           {
            work = 
               sin(PI*(mid_pt-n))*(filter_spec->GetMagRespSamp(num_taps/2));
            x = TWO_PI * (n-mid_pt)/num_taps;
            
            for(k=1; k<=(num_taps/2-1); k++)
              {
               work += (2.0*sin(x*k)*(filter_spec->GetMagRespSamp(k)));
              }
            Imp_Resp_Coeff[n] = work/num_taps;
           }
        }
      break;
   }
 for(n=0; n<num_taps; n++)
   {
    Original_Coeff[n] = Imp_Resp_Coeff[n];
   }
 return;
}
