//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  File = laguerre.cpp
//
//  Laguerre method for finding polynomial roots
//

#include <math.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include "complex.h"
#include "laguerre.h"

#ifdef _DEBUG
  extern std::ofstream DebugFile;
#endif

int LaguerreMethod( CmplxPolynomial *poly,
                    complex *root_ptr,
                    double epsilon,
                    double epsilon2,
                    int max_iter)
{
int iter, j, order;
complex p_eval, p_prime, p_double_prime;
complex root, f, f_sqrd, g, radical;
complex f_plus_rad, f_minus_rad, delta_root;
double old_delta_mag, root_mag, error;
complex *coeff;
order = poly->GetDegree();
coeff = new complex[order+1];
poly->CopyCoeffs(coeff);
root = *root_ptr;
old_delta_mag = 1000.0;

for(iter=1; iter<=max_iter; iter++)
  {
  p_double_prime = complex(0.0,0.0);
  p_prime = complex(0.0,0.0);
  p_eval = coeff[order];
  error = mag(p_eval);
  root_mag = mag(root);

  for( j=order-1; j>=0; j--)
    {
    p_double_prime = p_prime + root * p_double_prime;
    p_prime = p_eval + root * p_prime;
    p_eval = coeff[j] + root * p_eval;
    error = mag(p_eval) + root_mag * error;
    }
  error = epsilon2 * error;
  p_double_prime = 2.0 * p_double_prime;

  if(mag(p_eval) < error)
    {
    std::cout << "mag(p_eval) = " << mag(p_eval) << "  error = "
         << error << std::endl;
    *root_ptr = root;
    delete [] coeff;
    return(1);
    }
  f = p_prime/p_eval;
  f_sqrd = f * f;
  g = f_sqrd - p_double_prime/p_eval;
  radical = (order-1)*(order * g - f_sqrd);
  radical = sqrt(radical);
  f_plus_rad = f + radical;
  f_minus_rad = f - radical;
  if( mag(f_plus_rad) > mag(f_minus_rad) )
    {
    delta_root = complex(double(order), 0.0) / f_plus_rad;
    }
  else
    {
    delta_root = complex(double(order), 0.0) / f_minus_rad;
    }
  root = root - delta_root;
  if( (iter > 6) && (mag(delta_root) > old_delta_mag) )
    {
    *root_ptr = root;
    delete [] coeff;
    return(2);
    }
  if( mag(delta_root) < (epsilon * mag(root)))
    {
    *root_ptr = root;
    delete [] coeff;
    return(3);
    }
  old_delta_mag = mag(delta_root);
  }
#ifdef _DEBUG
  DebugFile << "Laguerre method failed to converge" << std::endl;
#endif
delete [] coeff;
return(-1);
}
