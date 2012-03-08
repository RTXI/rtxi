//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  File = cmpxpoly.cpp
//
//  class that implements a polynomial with
//  complex-valued coefficients
//

#include <math.h>
#include "cmpxpoly.h"
#include "laguerre.h"
#include "pause.h"
#include "stdlib.h"

//======================================================
//  default constructor

CmplxPolynomial::CmplxPolynomial( )
{
 Degree = 0;
 Coeff = new complex[1];
 Coeff[0] = complex( 0.0, 0.0);
 RemCoeff = NULL;
 Root = NULL;
 return;
};



//===================================================================
//  copy constructor

CmplxPolynomial::CmplxPolynomial( const CmplxPolynomial& original )
{
  int i;
  Degree = original.Degree;
  Coeff = new complex[Degree+1];
  Root = new complex[Degree];
 
  for( i=0; i<=Degree; i++)
    Coeff[i] = original.Coeff[i];

  for( i=0; i<Degree; i++)
    Root[i] = original.Root[i];

  return;
};



//===================================================================
//  constructor for initializing a binomial

CmplxPolynomial::CmplxPolynomial( const complex coeff_1,
                                  const complex coeff_0 )
{
 Degree = 1;
 Coeff = new complex[2];
 RemCoeff = NULL;
 Root = NULL;
 
 Coeff[0] = coeff_0;
 Coeff[1] = coeff_1;
 
 return;
}

//===================================================================
//   initializing constructor

CmplxPolynomial::CmplxPolynomial( const complex *coeff,
                                  const int degree )
{
 Degree = degree;
 Coeff = new complex[degree+1];
 RemCoeff = NULL;
 Root = NULL;
 
 for(int i=0; i<=degree; i++) Coeff[i] = coeff[i];
 for(int j=0; j<=degree; j++)
   {
   std::cout << "poly_coeff[" << j << "] = " << coeff[j] << std::endl;
   }
 
 return;
}

CmplxPolynomial::CmplxPolynomial( const double *coeff,
                                  const int degree )
{
 Degree = degree;
 Coeff = new complex[degree+1];
 RemCoeff = NULL;
 Root = NULL;
 
 for(int i=0; i<=degree; i++) Coeff[i] = complex(coeff[i],0.0);
 
 return;
}


//=================================================================
//  assignment operator

CmplxPolynomial& CmplxPolynomial::operator= (const CmplxPolynomial& right)
{
 if (Coeff != right.Coeff)
   {
   //-------------------------------------------------------------
   // Get rid of old coefficient array to make way for a new one
   // of the correct length for the new polynomial being assigned 
    
   delete [] Coeff;
   delete [] Root;
   
   Degree = right.Degree;
   Coeff = new complex[Degree+1];
    
   for( int i=0; i<=Degree; i++)
     Coeff[i] = right.Coeff[i];
   }
 return *this;
}


//===================================================================
// multiply assign operator        

CmplxPolynomial& CmplxPolynomial::operator*= (const CmplxPolynomial &right)
{
 //-----------------------------------------------------
 // save pointer to original coefficient array so that 
 // this array can be deleted once no longer needed
 
 complex *orig_coeff = Coeff;
 int orig_degree = Degree;
 
 //-------------------------------------------------------
 //  create new longer array to hold the new coefficients 
 
 Degree += right.Degree;
 Coeff = new complex[Degree+1];
 
 for( int i=0; i<=Degree; i++)
    Coeff[i] = complex(0.0, 0.0);
    
 //---------------------------------
 //  perform multiplication
 
 for( int rgt_indx=0; rgt_indx<= right.Degree; rgt_indx++)
   {
    for( int orig_indx=0; orig_indx <= orig_degree; orig_indx++)
      {
       Coeff[orig_indx+rgt_indx] +=
              (orig_coeff[orig_indx] * right.Coeff[rgt_indx]);
      }
   }

 return *this;
}    

//===================================================================
// divide assign operator        

CmplxPolynomial& CmplxPolynomial::operator/= (const CmplxPolynomial &divisor)
{
 //----------------------------------------------------
 //  In general, polynomial division will produce a
 //  quotient and a remainder.  This routine returns the
 //  quotient as its result.  The remainder will be
 //  stored in a member variable so that it can be
 //  checked or retrived by subsequent calls to the
 //  appropriate member functions.
 //-----------------------------------------------------
 // save pointer to original coefficient array so that 
 // this array can be deleted once no longer needed
 
 //complex *orig_coeff = Coeff;
 int dvdnd_deg, dvsr_deg, j, k;
 
 //-------------------------------------------------------
 //  create new array to hold the new coefficients 
 
 if(RemCoeff == NULL) RemCoeff = new complex[Degree+1];
 dvdnd_deg = Degree;
 dvsr_deg = divisor.Degree;
 Degree -= dvsr_deg;
    
 //---------------------------------
 //  perform division
 
  for(j=0; j<=dvdnd_deg; j++)
    {
    RemCoeff[j] = Coeff[j];
    Coeff[j] = complex(0.0,0.0);
    }
  for( k=dvdnd_deg-dvsr_deg; k>=0; k--)
    {
    Coeff[k] = RemCoeff[dvsr_deg+k]/divisor.Coeff[dvsr_deg];
    for(j=dvsr_deg+k-1; j>=k; j--)
      RemCoeff[j] -= Coeff[k]*divisor.Coeff[j-k];
    }
 for(j=dvsr_deg; j<=dvdnd_deg; j++)
                RemCoeff[j] = complex(0.0,0.0);
 return *this;
} 
//========================================================
//  Find roots of polynomial

void CmplxPolynomial::FindRoots( void )
{
  complex* root;
  int status, i;
  CmplxPolynomial root_factor;
  root = new complex[Degree];
  CmplxPolynomial work_poly;
  double epsilon=0.0000001;
  double epsilon2=1.0e-10;
  int max_iter=12;
  
  if(Root == NULL) Root = new complex[Degree];
  //------------------------------------------------
  // find coarse locations for roots

  work_poly = CmplxPolynomial(Coeff, Degree);

  for(i=0; i<Degree-1; i++)
    {
    Root[i] = complex(0.0,0.0);
    status = LaguerreMethod( &work_poly, &(Root[i]), 
                             epsilon, epsilon2, max_iter);
    if(status <0) 
      {
      std::cout << "Laguerre method did not converge" << std::endl;
      exit(55);
      }
    std::cout << "Root[" << i << "] = " << Root[i] 
         << " (" << status << ")" << std::endl;
    root_factor = CmplxPolynomial( complex(1.0,0.0),-Root[i]);
    work_poly /= root_factor;
    work_poly.DumpToStream(&std::cout);
    pausewait();
    }
  Root[Degree-1] = -(work_poly.GetCoeff(0));
  std::cout << "Root[" << Degree-1 << "] = " << Root[Degree-1] << std::endl;

  //------------------------------------------------
  //  polish the roots
  work_poly = CmplxPolynomial(Coeff, Degree);
  for(i=0; i<Degree; i++)
    {
    status = LaguerreMethod( &work_poly, &(Root[i]), 
                             epsilon, epsilon2, max_iter);
    if(status <0) 
      {
      std::cout << "Laguerre method did not converge" << std::endl;
      exit(55);
      }
    std::cout << "Polished Root[" << i << "] = " << Root[i] 
         << " (" << status << ")" << std::endl;
    pause();
    }
  return;
}

//========================================================
//  Get array of polynomial root values

complex* CmplxPolynomial::GetRoots( void )
{
  complex* root;
  int i;
  root = new complex[Degree];
  if(Root == NULL) this->FindRoots();

  for(i=0; i<Degree; i++) root[i] = Root[i];
  return(root);
}

//=========================================================
//  reflect root across the unit circle

void CmplxPolynomial::ReflectRoot( int root_idx )
{
  // (1) we must first find the roots (with more accuracy than
  // Laguerre alone provides) then (2) replace the root of
  // interest with its reciprocal and (3) reconstitute
  // the sum-of-powers coefficients from the revised set
  // of roots.
  this->FindRoots();
  Root[root_idx] = complex(1.0,0.0)/Root[root_idx];
  this->BuildFromRoots();
}
//=========================================================
//  build coefficients from the roots

void CmplxPolynomial::BuildFromRoots( void )
{
  if(Root != NULL)  // do nothing if roots not defined
    {
    }
}
//=========================================================
//  dump polynomial to an output stream

void CmplxPolynomial::DumpToStream( std::ostream* output_stream)
{
 (*output_stream) << "Degree = " << Degree << std::endl;
 
 for(int i=Degree; i>=0; i--)
   {
    (*output_stream) << "Coeff[" << i << "] = " 
                     << Coeff[i] << std::endl;
   }
 return;
}  


//====================================================
//

int CmplxPolynomial::GetDegree(void)
{
return(Degree);
}

//==================================================
//

complex CmplxPolynomial::GetCoeff(int k)
{
return Coeff[k];
}

//==================================================
//

void CmplxPolynomial::CopyCoeffs(complex *coeff)
{
for(int i=0; i<=Degree; i++) coeff[i] = Coeff[i];
return;
}

//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
