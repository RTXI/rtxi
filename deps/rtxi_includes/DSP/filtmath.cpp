//
//  file = filtmath.cpp
//
//  special functions for computing performance
//  of equiripple filters
//
#include <stdlib.h> 
#include <iostream> 
#include <fstream>
#include <math.h>
#include "misdefs.h"
#include "filtmath.h"

double DSubInf( double delta_p, double delta_s)
 {
  double log_delta_p, work;
  double a1 = 5.309e-3;
  double a2 = 7.114e-2;
  double a3 = -4.761e-1;
  double a4 = -2.66e-3;
  double a5 = -5.941e-1;
  double a6 = -4.278e-1;

  log_delta_p = log10(delta_p);

  work = a2 + a1 * log_delta_p;
  work *= log_delta_p;
  work += a3;
  work *= log10(delta_s);

  work += a4 * log_delta_p * log_delta_p;
  work += a5 * log_delta_p;
  work += a6;
  
  return work;
 }  

