//
//  File = con_rect.cpp
//

#include <math.h>
#include <iostream>
#include "misdefs.h"
#include "typedefs.h"
#include "con_rect.h"
#include "sinc.h"

ContRectangularMagResp::ContRectangularMagResp( istream& uin,
                                                ostream& uout )
                       : ContinWindowResponse( uin, uout )
{
 double tau, freq, freq_exp, value;
 double freq_cyc;
 
 tau = 1.0;
 freq_cyc = 2.0;
 
 for(int n=0; n<Num_Resp_Pts; n++)
   {
    freq_exp = 1.0 + freq_cyc*(n-Num_Resp_Pts)/((double)Num_Resp_Pts);
    freq = pow( (double)10.0, freq_exp);
    value = fabs(sinc(PI * freq * tau));
    if(Db_Scale_Enab) value = 20.0*log10(value);
    (*Response_File) << freq << ", " << value << std::endl; 
   }
 return;
}
                                         