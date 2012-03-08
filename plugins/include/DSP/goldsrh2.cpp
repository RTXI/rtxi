//
//  File = goldsrh2.cpp
//

#include "misdefs.h"
#include "typedefs.h"
#include "goldsrch.h"
#include "fs_util.h"
#include "fs_dsgn.h"
#include "sb_peak.h"
#include "fs_spec.h"
#include <fstream.h>
#include <math.h> 
#include <stdlib.h>

#ifdef _DEBUG
extern ofstream DebugFile;
#endif
extern ofstream LogFile;
extern logical PauseEnabled;

double GoldenSearch2( double tol,
                      FreqSampFilterSpec *filter_spec,
                      FreqSampFilterDesign *filter_design,
                      FreqSampFilterResponse *filter_resp,
                      double rho_min,
                      double rho_max,
                      double *origins,
                      double *slopes,
                      double *fmin)
{
 double x0, x1, x2, x3, xmin, f0, f1, f2, f3, oldXmin;
 double leftOrd, rightOrd, midOrd, x, xb;
 double trans_val;
 double delta;
 int n;
 logical db_scale;

 cout << "in goldenSearch\n" << endl;
 LogFile << "in goldenSearch\n" << endl;
 db_scale = TRUE; 

 /*--------------------------------------------*/
 filter_spec->SetTrans(origins, slopes, 0.0);
 filter_design->ComputeCoefficients(filter_spec);
 filter_resp->ComputeMagResp(filter_design, db_scale);
 filter_resp->NormalizeResponse(db_scale);
 leftOrd = filter_resp->GetStopbandPeak();
 LogFile << "leftOrd = " << leftOrd << endl;

 filter_spec->SetTrans(origins, slopes, 1.0);
 filter_design->ComputeCoefficients(filter_spec);
 filter_resp->ComputeMagResp(filter_design, db_scale);
 filter_resp->NormalizeResponse(db_scale);
 rightOrd = filter_resp->GetStopbandPeak();
 LogFile << "rightOrd = " << rightOrd << endl;
 pause(PauseEnabled);

if(leftOrd < rightOrd) {
  trans_val=1.0;
  for(;;) {
    trans_val = GOLD3 * trans_val;
    filter_spec->SetTrans(origins, slopes, trans_val);
    filter_design->ComputeCoefficients(filter_spec);
    filter_resp->ComputeMagResp(filter_design, db_scale);
    filter_resp->NormalizeResponse(db_scale);
    midOrd = filter_resp->GetStopbandPeak();
    LogFile << "midOrd = " << midOrd << endl;
    if(midOrd < leftOrd) break;
    }
  }
else {
  x = rho_max;
  for(;;) {
    x = GOLD3 * x;
    trans_val = rho_max - x;
    filter_spec->SetTrans(origins, slopes, trans_val);
    filter_design->ComputeCoefficients(filter_spec);
    filter_resp->ComputeMagResp(filter_design, db_scale);
    filter_resp->NormalizeResponse(db_scale);
    midOrd = filter_resp->GetStopbandPeak();
    LogFile << "midOrd = " << midOrd << endl;
    if(midOrd < rightOrd) break;
    }
  }

xb = trans_val;
/*--------------------------------------------*/
x0 = rho_min;
x3 = rho_max;
x1 = xb;
x2 = xb + GOLD3 * (rho_max - xb);
LogFile << "x0= " << x0 << " x1= " << x1
     << " x2= " << x2 << " x3= " << x3 << endl;

filter_spec->SetTrans(origins, slopes, x1);
filter_design->ComputeCoefficients(filter_spec);
filter_resp->ComputeMagResp(filter_design, db_scale);
filter_resp->NormalizeResponse(db_scale);
f1 = filter_resp->GetStopbandPeak();
LogFile << "f1 = " << f1 << endl;

filter_spec->SetTrans(origins, slopes, x2);
filter_design->ComputeCoefficients(filter_spec);
filter_resp->ComputeMagResp(filter_design, db_scale);
filter_resp->NormalizeResponse(db_scale);
f2 = filter_resp->GetStopbandPeak();
LogFile << "f2 = " << f2 << endl;

oldXmin = 0.0;

for(n=1; n<=100; n++) {
  if(f1<=f2) {
    x3 = x2;
    x2 = x1;
    x1 = GOLD6 * x2 + GOLD3 * x0;
    f3 = f2;
    f2 = f1;
    filter_spec->SetTrans(origins, slopes, x1);
    filter_design->ComputeCoefficients(filter_spec);
    filter_resp->ComputeMagResp(filter_design, db_scale);
    filter_resp->NormalizeResponse(db_scale);
    f1 = filter_resp->GetStopbandPeak();
    LogFile << "x0= " << x0 << " x1= " << x1
         << " x2= " << x2 << " x3= " << x3 << endl;
    LogFile << "f1 = " << f1 << endl;
    }
  else {
    x0 = x1;
    x1 = x2;
    x2 = GOLD6 * x1 + GOLD3 * x3;
    f0 = f1;
    f1 = f2;
    filter_spec->SetTrans(origins, slopes, x2);
    filter_design->ComputeCoefficients(filter_spec);
    filter_resp->ComputeMagResp(filter_design, db_scale);
    filter_resp->NormalizeResponse(db_scale);
    f2 = filter_resp->GetStopbandPeak();
    LogFile << "f2 = " << f2 << endl;
    LogFile << "x0= " << x0 << " x1= " << x1
         << " x2= " << x2 << " x3= " << x3 << endl;
    }

  delta = fabs(x3 - x0);
  oldXmin = xmin;
  LogFile << "at iter " << n << " delta = " << delta << endl;
  LogFile << "tol = " << tol << endl;
  if(delta <= tol) break;
  }
if(f1<f2)
  {xmin = x1;
  *fmin=f1;}
else
  {xmin = x2;
  *fmin=f2;}
cout << "minimum of " << *fmin << " at x = " << xmin << endl;
LogFile << "minimum of " << *fmin << " at x = " << xmin << endl;
return(xmin);
}

