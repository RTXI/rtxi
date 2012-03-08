//
//  File = goldsrch.cpp
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

double GoldenSearch( double tol,
                     FreqSampFilterSpec *filter_spec,
                     FreqSampFilterDesign *filter_design,
                     FreqSampFilterResponse *filter_resp,
                     long quant_factor,
                     double *fmin)
{
 double x0, x1, x2, x3, xmin, f0, f1, f2, f3, oldXmin;
 double leftOrd, rightOrd, midOrd, x, xb;
 double trans_val;
 double delta;
 int n;
 logical db_scale, rounding_enab;

 cout << "in goldenSearch\n" << endl;
 db_scale = TRUE;
 rounding_enab = FALSE;

 /*--------------------------------------------*/
 filter_spec->SetTrans(0.0);
 filter_design->ComputeCoefficients(filter_spec);
 filter_design->QuantizeCoefficients(quant_factor,rounding_enab);
 filter_resp->ComputeMagResp(filter_design, db_scale);
 filter_resp->NormalizeResponse(db_scale);
 leftOrd = filter_resp->GetStopbandPeak();
 cout << "leftOrd = " << leftOrd << endl;

 filter_spec->SetTrans(1.0);
 filter_design->ComputeCoefficients(filter_spec);
 filter_design->QuantizeCoefficients(quant_factor, rounding_enab);
 filter_resp->ComputeMagResp(filter_design, db_scale);
 filter_resp->NormalizeResponse(db_scale);
 rightOrd = filter_resp->GetStopbandPeak();
 cout << "rightOrd = " << rightOrd << endl;
 pause(PauseEnabled);

if(leftOrd < rightOrd) {
  trans_val=1.0;
  for(;;) {
    cout << "checkpoint 3" << endl;
    trans_val = GOLD3 * trans_val;
    filter_spec->SetTrans(trans_val);
    filter_design->ComputeCoefficients(filter_spec);
    filter_design->QuantizeCoefficients(quant_factor, rounding_enab);
    filter_resp->ComputeMagResp(filter_design, db_scale);
    filter_resp->NormalizeResponse(db_scale);
    midOrd = filter_resp->GetStopbandPeak();
    cout << "midOrd = " << midOrd << endl;
    if(midOrd < leftOrd) break;
    }
  }
else {
  x = 1.0;
  for(;;) {
    x = GOLD3 * x;
    trans_val = 1.0 - x;
    cout << "checkpoint 4" << endl;
    filter_spec->SetTrans(trans_val);
    filter_design->ComputeCoefficients(filter_spec);
    filter_design->QuantizeCoefficients(quant_factor, rounding_enab);
    filter_resp->ComputeMagResp(filter_design, db_scale);
    filter_resp->NormalizeResponse(db_scale);
    midOrd = filter_resp->GetStopbandPeak();
    cout << "midOrd = " << midOrd << endl;
    if(midOrd < rightOrd) break;
    }
  }

xb = trans_val;
/*--------------------------------------------*/
x0 = 0.0;
x3 = 1.0;
x1 = xb;
x2 = xb + GOLD3 * (1.0 - xb);
cout << "x0= " << x0 << " x1= " << x1
     << " x2= " << x2 << " x3= " << x3 << endl;

filter_spec->SetTrans(x1);
filter_design->ComputeCoefficients(filter_spec);
filter_design->QuantizeCoefficients(quant_factor, rounding_enab);
filter_resp->ComputeMagResp(filter_design, db_scale);
filter_resp->NormalizeResponse(db_scale);
f1 = filter_resp->GetStopbandPeak();

filter_spec->SetTrans(x2);
filter_design->ComputeCoefficients(filter_spec);
filter_design->QuantizeCoefficients(quant_factor, rounding_enab);
filter_resp->ComputeMagResp(filter_design, db_scale);
filter_resp->NormalizeResponse(db_scale);
f2 = filter_resp->GetStopbandPeak();

oldXmin = 0.0;

for(n=1; n<=100; n++) {
  if(f1<=f2) {
    x3 = x2;
    x2 = x1;
    x1 = GOLD6 * x2 + GOLD3 * x0;
    f3 = f2;
    f2 = f1;
    filter_spec->SetTrans(x1);
    filter_design->ComputeCoefficients(filter_spec);
    filter_design->QuantizeCoefficients(quant_factor, rounding_enab);
    filter_resp->ComputeMagResp(filter_design, db_scale);
    filter_resp->NormalizeResponse(db_scale);
    f1 = filter_resp->GetStopbandPeak();
    cout << "x0= " << x0 << " x1= " << x1
         << " x2= " << x2 << " x3= " << x3 << endl;
    }
  else {
    x0 = x1;
    x1 = x2;
    x2 = GOLD6 * x1 + GOLD3 * x3;
    f0 = f1;
    f1 = f2;
    filter_spec->SetTrans(x2);
    filter_design->ComputeCoefficients(filter_spec);
    filter_design->QuantizeCoefficients(quant_factor, rounding_enab);
    filter_resp->ComputeMagResp(filter_design, db_scale);
    filter_resp->NormalizeResponse(db_scale);
    f2 = filter_resp->GetStopbandPeak();
    cout << "x0= " << x0 << " x1= " << x1
         << " x2= " << x2 << " x3= " << x3 << endl;
    }

  delta = fabs(x3 - x0);
  oldXmin = xmin;
  cout << "at iter " << n << " delta = " << delta << endl;
  cout << "tol = " << tol << endl;
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

