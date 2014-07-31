//
//  File = hann.cpp
//

#include <math.h>
#include <iostream>
#include "typedefs.h"
#include "con_hann.h"
#include "misdefs.h"
#include "sinc.h"

ContHannMagResp::ContHannMagResp( istream& uin,
                                  ostream& uout )
                : ContinWindowResponse( uin, uout )
{
 double x, amp0, tau, freq, freq_exp, freq_cyc;
 
 tau = 1.0;
 freq_cyc = 2.0;
 amp0 = 0.5;

 for(int n=0; n<Num_Resp_Pts; n++)
   {
    freq_exp = 1.0 + freq_cyc*(n-Num_Resp_Pts)/((double)Num_Resp_Pts);
    freq = pow( (double)10.0, freq_exp);
    x = 0.5 * tau * sinc(PI * tau * freq);
    x += (0.25 * tau * sinc(PI * tau * (freq-tau)));
    x += (0.25 * tau * sinc(PI * tau * (freq+tau)));
    if(Db_Scale_Enab) x = 20.0*log10(fabs(x/amp0));
    (*Response_File) << freq << ", " << x << std::endl;
   }
}