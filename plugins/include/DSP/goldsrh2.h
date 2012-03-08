//
// File = goldsrh2.h
//


#ifndef _GOLDSRH2_H_
#define _GOLDSRH2_H_

#include "fs_spec.h"

double GoldenSearch2( double tol,
                      FreqSampFilterSpec *filter_spec,
                      FreqSampFilterDesign *filter_design,
                      FreqSampFilterResponse *filter_resp,
                      double rho_min,
                      double rho_max,
                      double *origins,
                      double *slopes,
                      double *fmin);
#endif 
