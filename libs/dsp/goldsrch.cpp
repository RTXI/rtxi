//
//  File = goldsrch.cpp
//

#include <fstream>
#include <math.h>
#include <stdlib.h>
#include <iostream>

#include "misdefs.h"
#include "typedefs.h"
#include "goldsrch.h"
#include "fs_util.h"
#include "fs_dsgn.h"
#include "sb_peak.h"
#include "fs_spec.h"

#ifdef _DEBUG
extern std::ofstream DebugFile;
#endif
// extern std::ofstream LogFile;
// extern logical PauseEnabled;

double
GoldenSearch(double tol, FreqSampFilterSpec* filter_spec,
             FreqSampFilterDesign* filter_design,
             FreqSampFilterResponse* filter_resp, long,
             double* fmin)
{
  double x0, x1, x2, x3, xmin, f1, f2;
  double leftOrd, rightOrd, midOrd, x, xb;
  double trans_val;
  double delta;
  int n;
  logical db_scale;

  std::cout << "in goldenSearch\n" << std::endl;
  db_scale = TRUE;
  //rounding_enab = FALSE;

  /*--------------------------------------------*/
  filter_spec->SetTrans(0.0);
  filter_design->ComputeCoefficients(filter_spec);
  filter_resp->ComputeMagResp(filter_design, db_scale);
  filter_resp->NormalizeResponse(db_scale);
  leftOrd = filter_resp->GetStopbandPeak();
  std::cout << "leftOrd = " << leftOrd << std::endl;

  filter_spec->SetTrans(1.0);
  filter_design->ComputeCoefficients(filter_spec);
  filter_resp->ComputeMagResp(filter_design, db_scale);
  filter_resp->NormalizeResponse(db_scale);
  rightOrd = filter_resp->GetStopbandPeak();
  std::cout << "rightOrd = " << rightOrd << std::endl;

  if (leftOrd < rightOrd) {
    trans_val = 1.0;
    for (;;) {
      std::cout << "checkpoint 3" << std::endl;
      trans_val = GOLD3 * trans_val;
      filter_spec->SetTrans(trans_val);
      filter_design->ComputeCoefficients(filter_spec);
      filter_resp->ComputeMagResp(filter_design, db_scale);
      filter_resp->NormalizeResponse(db_scale);
      midOrd = filter_resp->GetStopbandPeak();
      std::cout << "midOrd = " << midOrd << std::endl;
      if (midOrd < leftOrd)
        break;
    }
  } else {
    x = 1.0;
    for (;;) {
      x = GOLD3 * x;
      trans_val = 1.0 - x;
      std::cout << "checkpoint 4" << std::endl;
      filter_spec->SetTrans(trans_val);
      filter_design->ComputeCoefficients(filter_spec);
      filter_resp->ComputeMagResp(filter_design, db_scale);
      filter_resp->NormalizeResponse(db_scale);
      midOrd = filter_resp->GetStopbandPeak();
      std::cout << "midOrd = " << midOrd << std::endl;
      if (midOrd < rightOrd)
        break;
    }
  }

  xb = trans_val;
  /*--------------------------------------------*/
  x0 = 0.0;
  x3 = 1.0;
  x1 = xb;
  x2 = xb + GOLD3 * (1.0 - xb);
  std::cout << "x0= " << x0 << " x1= " << x1 << " x2= " << x2 << " x3= " << x3
            << std::endl;

  filter_spec->SetTrans(x1);
  filter_design->ComputeCoefficients(filter_spec);
  filter_resp->ComputeMagResp(filter_design, db_scale);
  filter_resp->NormalizeResponse(db_scale);
  f1 = filter_resp->GetStopbandPeak();

  filter_spec->SetTrans(x2);
  filter_design->ComputeCoefficients(filter_spec);
  filter_resp->ComputeMagResp(filter_design, db_scale);
  filter_resp->NormalizeResponse(db_scale);
  f2 = filter_resp->GetStopbandPeak();


  for (n = 1; n <= 100; n++) {
    if (f1 <= f2) {
      x3 = x2;
      x2 = x1;
      x1 = GOLD6 * x2 + GOLD3 * x0;
      f2 = f1;
      filter_spec->SetTrans(x1);
      filter_design->ComputeCoefficients(filter_spec);
      filter_resp->ComputeMagResp(filter_design, db_scale);
      filter_resp->NormalizeResponse(db_scale);
      f1 = filter_resp->GetStopbandPeak();
      std::cout << "x0= " << x0 << " x1= " << x1 << " x2= " << x2
                << " x3= " << x3 << std::endl;
    } else {
      x0 = x1;
      x1 = x2;
      x2 = GOLD6 * x1 + GOLD3 * x3;
      f1 = f2;
      filter_spec->SetTrans(x2);
      filter_design->ComputeCoefficients(filter_spec);
      filter_resp->ComputeMagResp(filter_design, db_scale);
      filter_resp->NormalizeResponse(db_scale);
      f2 = filter_resp->GetStopbandPeak();
      std::cout << "x0= " << x0 << " x1= " << x1 << " x2= " << x2
                << " x3= " << x3 << std::endl;
    }

    delta = fabs(x3 - x0);
    std::cout << "at iter " << n << " delta = " << delta << std::endl;
    std::cout << "tol = " << tol << std::endl;
    if (delta <= tol)
      break;
  }
  if (f1 < f2) {
    xmin = x1;
    *fmin = f1;
  } else {
    xmin = x2;
    *fmin = f2;
  }
  std::cout << "minimum of " << *fmin << " at x = " << xmin << std::endl;
  return (xmin);
}
