//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  File = poly.cpp
//
//  class that implements a polynomial with
//  real-valued coefficients
//

#include <math.h>
#include "poly.h"

//======================================================
//  default constructor

Polynomial::Polynomial( )
{
 Degree = 0;
 Coefficient = new double[1];
 Coefficient[0] = 0.0;
 return;
};



//===================================================================
//  copy constructor

Polynomial::Polynomial( const Polynomial& original )
{
 Degree = original.Degree;
 Coefficient = new double[Degree+1];
 
 for( int i=0; i<=Degree; i++)
   {
    Coefficient[i] = original.Coefficient[i];
   }
 return;
};



//===================================================================
//  conversion constructor

Polynomial::Polynomial( const CmplxPolynomial& original )
{
 Degree = original.Degree;
 Coefficient = new double[Degree+1];
 
 for( int i=0; i<=Degree; i++)
   {
    Coefficient[i] = real(original.Coeff[i]);
   }
 return;
};



//===================================================================
//  constructor for initializing a binomial

Polynomial::Polynomial( const double coeff_1,
                        const double coeff_0 )
{
 Degree = 1;
 Coefficient = new double[2];
 
 Coefficient[0] = coeff_0;
 Coefficient[1] = coeff_1;
 
 return;
}


//=================================================================
//  assignment operator

Polynomial& Polynomial::operator= (const Polynomial& right)
{
 if (Coefficient != right.Coefficient)
   {
    //-------------------------------------------------------------
    // Get rid of old coefficient array to make way for a new one
    // of the correct length for the new polynomial being assigned 
    
    delete [] Coefficient;
    
    Degree = right.Degree;
    Coefficient = new double[Degree+1];
    
    for( int i=0; i<=Degree; i++)
      {
       Coefficient[i] = right.Coefficient[i];
      }
   }
 return *this;
}


//===================================================================
// multiply assign operator        

Polynomial& Polynomial::operator*= (const Polynomial &right)
{
 //-----------------------------------------------------
 // save pointer to original coefficient array so that 
 // this array can be deleted once no longer needed
 
 double *orig_coeff = Coefficient;
 int orig_degree = Degree;
 
 //-------------------------------------------------------
 //  create new longer array to hold the new coefficients 
 
 Degree += right.Degree;
 Coefficient = new double[Degree+1];
 
 for( int i=0; i<=Degree; i++)
    Coefficient[i] = 0.0;
    
 //---------------------------------
 //  perform multiplication
 
 for( int rgt_indx=0; rgt_indx<= right.Degree; rgt_indx++)
   {
    for( int orig_indx=0; orig_indx <= orig_degree; orig_indx++)
      {
       Coefficient[orig_indx+rgt_indx] +=
              (orig_coeff[orig_indx] * right.Coefficient[rgt_indx]);
      }
   }

 return *this;
}    

//===================================================================
// divide assign operator        

Polynomial& Polynomial::operator/= (const Polynomial &right)
{
 //-----------------------------------------------------
 // save pointer to original coefficient array so that 
 // this array can be deleted once no longer needed
 
 double *orig_coeff = Coefficient;
 int orig_degree = Degree;
 
 //-------------------------------------------------------
 //  create new longer array to hold the new coefficients 
 
 Degree += right.Degree;
 Coefficient = new double[Degree+1];
 
 for( int i=0; i<=Degree; i++)
    Coefficient[i] = 0.0;
    
 //---------------------------------
 //  perform multiplication
 
 for( int rgt_indx=0; rgt_indx<= right.Degree; rgt_indx++)
   {
    for( int orig_indx=0; orig_indx <= orig_degree; orig_indx++)
      {
       Coefficient[orig_indx+rgt_indx] +=
              (orig_coeff[orig_indx] * right.Coefficient[rgt_indx]);
      }
   }

 return *this;
}    


//=========================================================
//  dump polynomial to an output stream

void Polynomial::DumpToStream( std::ofstream* output_stream)
{
 (*output_stream) << "Degree = " << Degree << std::endl;
 
 for(int i=Degree; i>=0; i--)
   {
    (*output_stream) << "Coeff[" << i << "] = " 
                     << Coefficient[i] << std::endl;
   }
 return;
}  



//====================================================
//

int Polynomial::GetDegree(void)
{
return(Degree);
}

//==================================================
//

double Polynomial::GetCoefficient(int k)
{
return Coefficient[k];
}

//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
