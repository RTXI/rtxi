//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  File = bessfunc.cpp
//
//  Bessel Filter Function
//

#include <math.h>
#include <stdlib.h>
#include "bessfunc.h"
#include "elipfunc.h"
#include "laguerre.h"
#include "complex.h"
#include "cmpxpoly.h"
#ifdef _DEBUG
extern std::ofstream DebugFile;
#endif

//======================================================
//  constructor

BesselTransFunc::BesselTransFunc( int order, 
                                  double passband_edge,
                                  int norm_for_delay)
                :FilterTransFunc(order)
{
  int indx, indx_m1, indx_m2;
  int i, n, ii, work_order;
  double epsilon, epsilon2;
  int max_iter, laguerre_status;
  long q_poly[3][MAX_BESSEL_ORDER];
  complex *denom_poly, *work_coeff;
  complex root, work1, work2;
  double renorm_val, smallest;
  CmplxPolynomial *work_poly;

  //-------------------------------------------------------
  //  these values are reciprocals of values in Table 5.10

  double renorm_factor[9] = { 0.0,     0.0,     0.72675, 
                              0.57145, 0.46946, 0.41322, 
                              0.37038, 0.33898, 0.31546};

 Prototype_Pole_Locs = new complex[order+1];
 Num_Prototype_Poles = order;
 Prototype_Zero_Locs = new complex[1];
 Num_Prototype_Zeros = 0;
 
 H_Sub_Zero = 1.0;
  denom_poly = new complex[MAX_BESSEL_ORDER];
  work_coeff = new complex[MAX_BESSEL_ORDER];

  indx = 1;
  indx_m1 = 0;
  indx_m2 = 2;
  renorm_val = renorm_factor[order];

  //-----------------------------------------
  // initialize polynomials for n=0 and n=1

  for( i=0; i<(3*MAX_BESSEL_ORDER) ; i++) 
      q_poly[0][i] = 0;

  q_poly[0][0] = 1;
  q_poly[1][0] = 1;
  q_poly[1][1] = 1;

  //----------------------------------------------
  //  compute polynomial using recursion from n=2
  //  up through n=order

  for( n=2; n<=order; n++)
    {
    indx = (indx+1)%3;
    indx_m1 = (indx_m1+1)%3;
    indx_m2 = (indx_m2+1)%3;

    for( i=0; i<n; i++)
      {
      q_poly[indx][i] = (2*n-1) * 
                             q_poly[indx_m1][i];
      }
    for( i=2; i<=n; i++)
      {
      q_poly[indx][i] = q_poly[indx][i] + 
                             q_poly[indx_m2][i-2];
      }
    }
  if(norm_for_delay)
    {
    for( i=0; i<=order; i++)
      {
      denom_poly[i] = complex(
                      double(q_poly[indx][i]), 0.0);
      #ifdef _DEBUG
      DebugFile << "q_poly[" << i << "] = "
                << q_poly[indx][i] << std::endl;
      #endif
      }
    }
  else
    {
    for( i=0; i<=order; i++)
      denom_poly[i] = complex(
                      (double(q_poly[indx][i]) * 
                      ipow(renorm_val, (order-i))), 0.0);
    }
  //---------------------------------------------------
  // use Laguerre method to find roots of the
  // denominator polynomial -- these roots are the
  // poles of the filter

  epsilon = 1.0e-6;
  epsilon2 = 1.0e-6;
  max_iter = 10;

  for(i=0; i<=order; i++) work_coeff[i] = denom_poly[i];

  for(i=order; i>1; i--)
    {
    root = complex(0.0,0.0);
    work_order = i;
    work_poly = new CmplxPolynomial( work_coeff, work_order);
    laguerre_status = LaguerreMethod( work_poly,
                                      &root,
                                      epsilon,
                                      epsilon2,
                                      max_iter);
    delete work_poly;

    #ifdef _DEBUG
    DebugFile << "laguerre_status = "
              << laguerre_status << std::endl;
    #endif

    if(laguerre_status <0)
      {
      #ifdef _DEBUG
      DebugFile << "FATAL ERROR - \n"
                << "Laguerre method failed to converge.\n"
                << "Unable to find poles for desired Bessel filter." 
                << std::endl;
      #endif
      exit(-1);
      }

    #ifdef _DEBUG
    DebugFile << "root = " << root << std::endl;
    #endif

    //--------------------------------------------
    // if imaginary part of root is very small
    // relative to real part, set it to zero

    if(fabs(imag(root)) < epsilon*fabs(real(root)))
      {
      root = complex(real(root), 0.0);
      }
    Prototype_Pole_Locs[order+1-i] = root;

    //---------------------------------------------
    // deflate working polynomial by removing 
    // (s - r) factor where r is newly found root

    work1 = work_coeff[i];
    for(ii=i-1; ii>=0; ii--)
      {
      work2 = work_coeff[ii];
      work_coeff[ii] = work1;
      work1 = work2 + root * work1;
      }
    } // end of loop over i
  #ifdef _DEBUG
  DebugFile << "work_coeff[1] = " << work_coeff[1] << std::endl;
  DebugFile << "work_coeff[0] = " << work_coeff[0] << std::endl;
  #endif
  
  Prototype_Pole_Locs[order] = -work_coeff[0];

  #ifdef _DEBUG
  DebugFile << "pole[" << order << "] = "
            << Prototype_Pole_Locs[order] << std::endl;
  #endif
  //----------------------------------------------
  // sort poles so that imaginary parts are in
  // ascending order.  This order is critical for
  // sucessful operation of ImpulseResponse().

  for(i=1; i<order; i++)
    {
    smallest = imag(Prototype_Pole_Locs[i]);
    for( ii=i+1; ii<=order; ii++)
      {
      if(smallest <= imag(Prototype_Pole_Locs[ii])) continue;
        work1 = Prototype_Pole_Locs[ii];
        Prototype_Pole_Locs[ii] = Prototype_Pole_Locs[i];
        Prototype_Pole_Locs[i] = work1;
        smallest = imag(work1);
      }
    }

  #ifdef _DEBUG
  for(i=1; i<=order; i++)
    {
    DebugFile << "pole[" << i << "] = " 
              << Prototype_Pole_Locs[i] << std::endl;
    }
  #endif

  return;
}