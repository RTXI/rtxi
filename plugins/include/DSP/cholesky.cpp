 //
 //  File = cholesky.cpp
 //

#include <fstream>
#include "complex.h"
#include "cholesky.h"

#ifdef _DEBUG
  extern std::ofstream DebugFile;
#endif

 int CholeskyDecomp( int ord,
                     matrix<complex> *ax,
                     complex *bx,
                     double epsilon )
 {
  double *dx;
  int i, j, k;
  complex *yx;
  matrix<complex> lx(1,ord, 1, ord);

  dx = new double[ord+1];
  yx = new complex[ord+1];

  dx[1] = real( (*ax)[1][1] );
  for(i=2; i<=ord; i++)
    {
    for(j=1; j<i; j++)
      {
      lx[i][j] = conj( (*ax)[j][i] ) / dx[j];
      if(j==1) continue;

      for(k=1; k<j; k++)
        {
        lx[i][j] -= lx[i][k] * dx[k] * conj(lx[j][k])/dx[j];
        } // end of loop over k
      } // end of loop over j
    dx[i] = real( (*ax)[i][i]);

    for(k=1; k<i; k++)
      {
      dx[i] -= dx[k] * mag_sqrd( lx[i][k] );
      }
    //
    // Non-positive dx[i] is an error condition most likely
    // caused by ill-conditioned ax matrix

    if( dx[i] < 0.0 )
//    if( dx[i] < epsilon )
      {
      delete[] dx;
      delete[] yx;
      return(-1);
      }
    } // end of loop over i

  //- - - - - - - - - - - - - - - - - - 
  //  Solve for y vector in Eq. (31.x)
  //
  yx[1] = bx[1];
  for(k=2; k<=ord; k++)
    {
    yx[k] = bx[k];

    for(j=1; j<k; j++)
      {
      yx[k] -= lx[k][j] * yx[j];
      }
    } // end of loop over k

  //- - - - - - - - - - - - - - - - - - -
  //  Solve for x vector in Eq. (31.x)
  //
  bx[ord] = yx[ord]/dx[ord];

  for(k=ord-1; k>=1; k--)
    {
    bx[k] = yx[k]/dx[k];

    for(j=k+1; j<=ord; j++)
      {
      bx[k] -= conj(lx[j][k]) * bx[j];
      }
    }
  delete[] dx;
  delete[] yx;
  return(0);
 }
 
