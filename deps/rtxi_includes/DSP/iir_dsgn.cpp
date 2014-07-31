//
//  File = iir_dsgn.cpp
//

#include <math.h>
#include <stdlib.h>
#include <iostream>
#include "misdefs.h"   
#include "iir_dsgn.h"
#ifdef _DEBUG
extern std::ofstream DebugFile;
#endif

//=========================================
// default constructor
//-----------------------------------------

IirFilterDesign::IirFilterDesign( void )
{
 return;
} 
//======================================================
// constructor that provides interactive initialization
//------------------------------------------------------

IirFilterDesign::IirFilterDesign( std::istream& uin,
                                  std::ostream& uout)
{
 uout << "We made it into interactive constructor" << std::endl;
}
//===============================================
//  constructor that allocates arrays of
//  length num_taps to hold coefficients
//-----------------------------------------------

IirFilterDesign::IirFilterDesign( int num_numer_coeffs,
                                  int num_denom_coeffs )
{
 Num_Numer_Coeffs = num_numer_coeffs;
 Num_Denom_Coeffs = num_denom_coeffs;
 Numer_Coeffs = new double[num_numer_coeffs];
 Denom_Coeffs = new double[num_denom_coeffs+1];
 Orig_Numer_Coeffs = new double[num_numer_coeffs];
 Orig_Denom_Coeffs = new double[num_denom_coeffs+1];
}


//==========================================================
// constructor that allocates array of length num_taps 
// and initializes this array to values contained in 
// input array *imp_resp_coeff 
//----------------------------------------------------------

IirFilterDesign::IirFilterDesign( int num_numer_coeffs,
                                  int num_denom_coeffs,
                                  double *numer_coeffs,
                                  double *denom_coeffs)
{
 int n;
 Num_Numer_Coeffs = num_numer_coeffs;
 #ifdef _DEBUG
 DebugFile << "in IirFilterDesign, num_numer_coeffs = "
           << num_numer_coeffs << std::endl;
 #endif
 Num_Denom_Coeffs = num_denom_coeffs;
 #ifdef _DEBUG
 DebugFile << "in IirFilterDesign, num_denom_coeffs = "
           << num_denom_coeffs << std::endl;
 #endif
 Numer_Coeffs = new double[num_numer_coeffs];
 Denom_Coeffs = new double[num_denom_coeffs+1];
 Orig_Numer_Coeffs = new double[num_numer_coeffs];
 Orig_Denom_Coeffs = new double[num_denom_coeffs+1];
 
 for(n=0; n<num_numer_coeffs; n++)
   {
    Numer_Coeffs[n] = numer_coeffs[n];
    Orig_Numer_Coeffs[n] = numer_coeffs[n];
   }
 Denom_Coeffs[0] = 0.0;
 Orig_Denom_Coeffs[0] = 0.0;
 for(n=1; n<=num_denom_coeffs; n++)
   {
    Denom_Coeffs[n] = denom_coeffs[n];
    Orig_Denom_Coeffs[n] = denom_coeffs[n];
   }
 return;
} 


//============================================== 
// method to allocate coefficient arrays 
// after default constructor has been used 
//----------------------------------------------

void IirFilterDesign::Initialize( int num_numer_coeffs,
                                  int num_denom_coeffs )
{
 Num_Numer_Coeffs = num_numer_coeffs;
 Num_Denom_Coeffs = num_denom_coeffs;
 Numer_Coeffs = new double[num_numer_coeffs];
 Denom_Coeffs = new double[num_denom_coeffs+1];
 Orig_Numer_Coeffs = new double[num_numer_coeffs];
 Orig_Denom_Coeffs = new double[num_denom_coeffs+1];
}

//============================================================
//  method to quantize coefficients
//------------------------------------------------------------

void IirFilterDesign::QuantizeCoefficients( long quant_factor,
                                            logical rounding_enabled )
{
 int n;
 long work_long;
 double quan_err;
 
 //-----------------------------------
 // if quant_factor == 0, then restore
 // coefficients to their original,
 // unquantized values
 
 if( quant_factor == 0)
   {
    for( n=0; n<Num_Numer_Coeffs; n++)
      {
       Numer_Coeffs[n] = Orig_Numer_Coeffs[n];
      }
    for( n=1; n<=Num_Denom_Coeffs; n++)
      {
       Denom_Coeffs[n] = Orig_Denom_Coeffs[n];
      }
    return;
   }

 //-------------------------------------------
 // quantize the original coefficient values
    
 for( n=0; n< Num_Numer_Coeffs; n++)
  {
   if(rounding_enabled)
     {work_long = long((quant_factor * Orig_Numer_Coeffs[n])+0.5);}
   else
     {work_long = long(quant_factor * Orig_Numer_Coeffs[n]);}
     
   Numer_Coeffs[n] = double(work_long)/double(quant_factor);
   quan_err = (Numer_Coeffs[n] - Orig_Numer_Coeffs[n])
              / Orig_Numer_Coeffs[n];
   #ifdef _DEBUG
   DebugFile << "numer coeff " << n << " quant from "
             << Orig_Numer_Coeffs[n] << "\n    to "
             << Numer_Coeffs[n] << " error is "
             << quan_err << std::endl;
   #endif
  }
 for( n=1; n<= Num_Denom_Coeffs; n++)
  {
   if(rounding_enabled)
     {work_long = long((quant_factor * Orig_Denom_Coeffs[n])+0.5);}
   else
     {work_long = long(quant_factor * Orig_Denom_Coeffs[n]);}
     
   Denom_Coeffs[n] = double(work_long)/double(quant_factor);
   quan_err = (Denom_Coeffs[n] - Orig_Denom_Coeffs[n])
              / Orig_Denom_Coeffs[n];
   #ifdef _DEBUG
   DebugFile << "denom coeff " << n << " quant from "
             << Orig_Denom_Coeffs[n] << "\n   to "
             << Denom_Coeffs[n] << " error is "
             << quan_err << std::endl;
   #endif
  }
 return;
}       


//============================================================
//  method to scale coefficients
//------------------------------------------------------------

void IirFilterDesign::ScaleCoefficients( double scale_factor )
{
 int n;
 for( n=0; n< Num_Numer_Coeffs; n++)
  {
   Orig_Numer_Coeffs[n] = scale_factor * Orig_Numer_Coeffs[n];
   Numer_Coeffs[n] = Orig_Numer_Coeffs[n];
  }
 for( n=1; n<= Num_Denom_Coeffs; n++)
  {
   Orig_Denom_Coeffs[n] = scale_factor * Orig_Denom_Coeffs[n];
   Denom_Coeffs[n] = Orig_Denom_Coeffs[n];
  }
 return;
}       

//======================================================
// copy coefficient values from internal arrays
// to output arrays *numer_coeff and *denom_coeff
//------------------------------------------------------
 
void IirFilterDesign::CopyCoefficients( double *numer_coeff,
                                        double *denom_coeff)
{
 int n;
 for(n=0; n<Num_Numer_Coeffs; n++)
   {
    numer_coeff[n] = Numer_Coeffs[n];
   }
 for(n=0; n<=Num_Denom_Coeffs; n++)
   {
    denom_coeff[n] = Denom_Coeffs[n];
   }
 return;
}

//==============================================
//  get pointer to numerator coefficient array
//----------------------------------------------
double* IirFilterDesign::GetNumerCoefficients(void)
{
 std::cout << "in iir_dsgn, Numer_Coeffs = " << (void*)Numer_Coeffs << std::endl;
 return(Numer_Coeffs);
}


//==============================================
//  get a copy of the denominator coeffcients
//----------------------------------------------
double* IirFilterDesign::GetDenomCoefficients(void)
{
 std::cout << "in iir_dsgn, Denom_Coeffs = " 
      << (void*)Denom_Coeffs << std::endl;
 double* ret_ptr = new double[Num_Denom_Coeffs+1];
 for(int i=0; i<=Num_Denom_Coeffs; i++)
   ret_ptr[i] = Denom_Coeffs[i];
 return(ret_ptr);
}

//==============================================
//  set the denominator coeffcients
//----------------------------------------------
void IirFilterDesign::SetDenomCoefficients( int num_coeffs,
                                            double *coeffs)
{
 Num_Denom_Coeffs = num_coeffs;
 delete[] Denom_Coeffs;
 Denom_Coeffs = new double[num_coeffs+1];
 for(int i=0; i<=Num_Denom_Coeffs; i++)
   Denom_Coeffs[i] = coeffs[i];
 return;
}

//==========================================
void IirFilterDesign::SetSamplingInterval(double samp_intvl)
{
 Sampling_Interval = samp_intvl;
 return;
}

double IirFilterDesign::GetSamplingInterval(void)
{
 return(Sampling_Interval);
}

//==========================================
//  get number of numerator coefficients
//------------------------------------------
 
int IirFilterDesign::GetNumNumerCoeffs(void)
{
 return(Num_Numer_Coeffs);
}


//==========================================
//  get number of denominator coefficients
//------------------------------------------
 
int IirFilterDesign::GetNumDenomCoeffs(void)
{
 return(Num_Denom_Coeffs);
}


//==============================================================
// dump complete set of coefficients to output_stream
//--------------------------------------------------------------

void IirFilterDesign::DumpCoefficients( std::ofstream* output_stream)
{
 int n; 
 //output_stream->setf(ios::fixed, ios::floatfield);
 //output_stream->precision(11);
 for(n=0; n<Num_Numer_Coeffs; n++)
   {
    (*output_stream) << "b[" << n << "] = " 
                     << Numer_Coeffs[n] << std::endl;
   }
 for(n=1; n<=Num_Denom_Coeffs; n++)
   {
    (*output_stream) << "a[" << n << "] = " 
                     << Denom_Coeffs[n] << std::endl;
   }
 //output_stream->precision(0);
 //output_stream->setf(0, ios::floatfield);
 return;
}



