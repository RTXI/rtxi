//
//  File = covmeth.cpp
//

#include <stdlib.h>
#include <fstream>
#include "covmeth.h"

#ifdef _DEBUG
  extern std::ofstream DebugFile;
#endif

CovarMethCorrMtx::CovarMethCorrMtx( complex *x,
                                    int seq_len,
                                    int max_lag)
                : matrix<complex>(1,max_lag+1,1,max_lag+1)
{
  int i, j, n, p, i_idx, j_idx;
  complex sum;
  double denom;

  p = max_lag +1;
  denom = double(seq_len - p);

  for(i=1; i<=p; i++)
    {
    for(j=1; j<=p; j++)
      {
      i_idx = p-i;
      j_idx = p-j;
      sum = complex(0.0,0.0);
      for(n=p; n<seq_len; n++)
        {
        sum += conj(x[i_idx]) * x[j_idx];
        i_idx++;
        j_idx++;
        } // end of loop over n
      sum = sum/denom;
      (*(_p->f[i-1]))[j] = sum;
      }  // end of loop over j
    } // end of loop over i

  return;
}

//==============================================
complex* CovarMethRightHandVect( complex *x,
                                 int seq_len,
                                 int max_lag)
{
  int i, n, p, i_idx, j_idx;
  complex sum, *r;
  double denom;

  p = max_lag +1;
  denom = double(seq_len - p);

  r = new complex[p+1];
  r[0] = complex(0.0,0.0);

  for(i=1; i<=p; i++)
    {
    i_idx = p-i;
    j_idx = p;
    sum = complex(0.0,0.0);
    for(n=p; n<seq_len; n++)
      {
      sum += conj(x[i_idx]) * x[j_idx];
      i_idx++;
      j_idx++;
      } // end of loop over n
    sum = sum/denom;
    r[i] = sum;
    } // end of loop over i

  return(r);
}

