 //
 //  File = gen_lev.cpp
 //

#include "complex.h"
#include <fstream>
#include "matrix_T.h"
#include "gen_lev.h"
#include "overload.h"

 int GeneralizedLevinson( double *acf,
                          int ar_ord,
                          int ma_ord,
                          double epsilon,
                          double *a_vec)
 {
  matrix<double> ax(1, ar_ord, 1, ar_ord);
  matrix<double> bx(1, ar_ord, 1, ar_ord);
  double rho, cx, dx;
  int i, k, m, n, lag;

  // Algorithm 22.3 step 1
  ax[1][1] = -acf[ma_ord+1]/acf[ma_ord];
  if(ar_ord > 1)
    {
    if(ma_ord == 0)
        bx[1][1] = -(conj(acf[1])/acf[ma_ord]);
    else
        bx[1][1] = -acf[ma_ord-1]/acf[ma_ord];

    rho = (1.0 - ax[1][1]*bx[1][1]) * acf[ma_ord];

    if(mag(rho) <= epsilon)  
             return(-1);  // initial 2 x 2 matrix is singular

    //----------------------------------
    //  main loop starts here
    //  Algorithm 22.3 step 2

    for(k=2; k<=ar_ord; k++)
      {
      //  compute (22.19)
      cx = -acf[ma_ord + k];
      for( m=1; m<k; m++)
        {
        lag = ma_ord + k - m;
        if(lag >= 0)
          cx -= (acf[lag] * ax[m][k-1]);
        else
          cx -= (conj(acf[-lag]) * ax[m][k-1]);
        } // end of loop over m
      ax[k][k] = cx/rho;

      // update ax[i]'s using Eq.(22.20)
      for(i=1; i<k; i++)
        ax[i][k] = ax[i][k-1] + ax[k][k] * bx[k-i][k-1];

      if(k < ar_ord)
        {
        // compute bx[k] Eq.(22.21)
        lag = ma_ord - k;
        if(lag >=0)
          dx = -acf[lag];
        else
          dx = -conj(acf[-lag]);

        for(n=1; n<k; n++)
          {
          lag = ma_ord - k + n;
          //lag = ma_ord - k - n;
          if(lag >= 0)
            dx -= (acf[lag] * bx[n][k-1]);
          else
            dx -= (conj(acf[-lag]) * bx[n][k-1]);
          } // end of loop over n
        bx[k][k] = dx/rho;

        // update bx[i]'s Eq.(22.22)
        for(i=1; i<k; i++)
          bx[i][k] = bx[i][k-1] + bx[k][k] * ax[k-i][k-1];

        rho *= (1.0 - ax[k][k] * bx[k][k]);

        if(mag(rho) <= epsilon)
               return(-1);  // (k+1) x (k+1) matrix is singular
 
        } // end if if(k < ar_ord)
      }  // end of loop over k
    } // end of if(ar_ord >1)

  // copy result from working matrix into output array
  for(i=1; i<=ar_ord; i++)
    a_vec[i] = ax[i][ar_ord];
  a_vec[0] = 1.0;
  return(0);
 };
 //================================================
 int GeneralizedLevinson( complex *acf,
                          int ar_ord,
                          int ma_ord,
                          double epsilon,
                          complex *a_vec)
 {
  matrix<complex> ax(1, ar_ord, 1, ar_ord);
  matrix<complex> bx(1, ar_ord, 1, ar_ord);
  complex rho, cx, dx;
  int i, k, m, n, lag;

  // Algorithm 22.3 step 1
  ax[1][1] = -acf[ma_ord+1]/acf[ma_ord];
  if(ar_ord > 1)
    {
    if(ma_ord == 0)
        bx[1][1] = -(conj(acf[1])/acf[ma_ord]);
    else
        bx[1][1] = -acf[ma_ord-1]/acf[ma_ord];

    rho = (1.0 - ax[1][1]*bx[1][1]) * acf[ma_ord];

    if(mag(rho) <= epsilon)  
             return(-1);  // initial 2 x 2 matrix is singular

    //----------------------------------
    //  main loop starts here
    //  Algorithm 22.3 step 2

    for(k=2; k<=ar_ord; k++)
      {
      //  compute Eq.(22.19)
      cx = -acf[ma_ord + k];
      for( m=1; m<k; m++)
        {
        lag = ma_ord + k - m;
        if(lag >= 0)
          cx -= (acf[lag] * ax[m][k-1]);
        else
          cx -= (conj(acf[-lag]) * ax[m][k-1]);
        } // end of loop over m
      ax[k][k] = cx/rho;

      // update ax[i]'s using Eq.(22.20)
      for(i=1; i<k; i++)
        ax[i][k] = ax[i][k-1] + ax[k][k] * bx[k-i][k-1];

      if(k < ar_ord)
        {
        // compute bx[k] Eq.(22.21)
        lag = ma_ord - k;
        if(lag >=0)
          dx = -acf[lag];
        else
          dx = -conj(acf[-lag]);

        for(n=1; n<k; n++)
          {
          lag = ma_ord - k + n;
          //lag = ma_ord - k - n; 
          if(lag >= 0)
            dx -= (acf[lag] * bx[n][k-1]);
          else
            dx -= (conj(acf[-lag]) * bx[n][k-1]);
          } // end of loop over n
        bx[k][k] = dx/rho;

        // update bx[i]'s (22.22)
        for(i=1; i<k; i++)
          bx[i][k] = bx[i][k-1] + bx[k][k] * ax[k-i][k-1];

        rho *= (1.0 - ax[k][k] * bx[k][k]);

        if(mag(rho) <= epsilon)
               return(-1);  // (k+1) x (k+1) matrix is singular
 
        } // end if if(k < ar_ord)
      }  // end of loop over k
    } // end of if(ar_ord >1)

  // copy result from working matrix into output array
  for(i=1; i<=ar_ord; i++)
    a_vec[i] = ax[i][ar_ord];
  a_vec[0] = 1.0;
  return(0);
 };
