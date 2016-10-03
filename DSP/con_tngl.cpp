//
//  File = con_tngl.cpp
//

#include <math.h>
#include <iostream>
#include "con_tngl.h"
#include "misdefs.h"
#include "typedefs.h"
#include "sincsqrd.h"

ContTriangularMagResp::ContTriangularMagResp( istream& uin,
                                              ostream& uout )
                      :ContinWindowResponse( uin, uout )
{
 double amp0, x, value;
 double tau, freq, freq_exp, freq_cyc;
 
 tau = 1.0;
 freq_cyc = 2.0;

 for(int n=0; n<Num_Resp_Pts; n++)
   {
    freq_exp = 1.0 + freq_cyc*(n-Num_Resp_Pts)/((double)Num_Resp_Pts);
    freq = pow( (double)10.0, freq_exp);
    amp0 = 0.5 * tau;
    x = PI * freq * tau /2.0;
    value = 0.5 * tau * sinc_sqrd(x) / amp0;
    if(Db_Scale_Enab) value = 20.0*log10(value);
    (*Response_File) << freq << ", " << value << std::endl;
   }
 
}