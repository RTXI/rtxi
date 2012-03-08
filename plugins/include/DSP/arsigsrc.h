//
//  File = arsigsrc.h
//

#ifndef _ARSIGSRC_H_
#define _ARSIGSRC_H_
#include "complex.h"

void ArSignalSource( istream& uin, ostream& uout, 
                     double *ret_samp_intvl, 
                     int seq_len, 
                     complex *signal);

#endif                               
