//
//  File = optmiz2.h
//

#ifndef _OPTMIZ2_H_
#define _OPTMIZ2_H_

#include "fs_spec.h"
#include "fs_dsgn.h"
#include "fs_resp.h"

void optimize2( FreqSampFilterSpec *filter_spec,
                FreqSampFilterDesign *filter_design,
                FreqSampFilterResponse *filter_resp,
                double y_base_init,
                double tol,
                double tweakFactor,
                double rectComps[]);


#endif
