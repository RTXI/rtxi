//
//  File = unq_iir.cpp
//

#include <stdlib.h>
#include <fstream>
#include "unq_iir.h"
#ifdef _DEBUG
extern std::ofstream DebugFile;
#endif

UnquantDirectFormIir::UnquantDirectFormIir( 
                              int num_numer_coeff,
                              int num_denom_coeff,
                              double *numer_coeff,
                              double *denom_coeff)
{
 int n;
 Num_Numer_Coeff = num_numer_coeff;
 Num_Denom_Coeff = num_denom_coeff;
 Input_Buffer = new double[num_numer_coeff];
 Output_Buffer = new double[num_denom_coeff+1];
 Numer_Coeff = new double[num_numer_coeff];
 Denom_Coeff = new double[num_denom_coeff+1];
 Input_Write_Indx = 0;
 Output_Write_Indx = 1;
 
 #ifdef _DEBUG
 DebugFile << "In UnquantDirectFormIir" << std::endl;
 #endif
 
 for(n=0; n<num_numer_coeff; n++)
   {
    Numer_Coeff[n] = numer_coeff[n];
    Input_Buffer[n] = 0.0;
   }
 for(n=1; n<=num_denom_coeff; n++)
   {
    Denom_Coeff[n] = denom_coeff[n];
    Output_Buffer[n] = 0.0;
   }
 return;
}

double UnquantDirectFormIir::ProcessSample( double input_val )
{
 double output_val;
 double term, sum;
 int input_read_indx, output_read_indx, tap_indx;
 
 Input_Buffer[Input_Write_Indx] = input_val;
 input_read_indx = Input_Write_Indx;
 Input_Write_Indx++;
 if(Input_Write_Indx >= Num_Numer_Coeff) 
                                   Input_Write_Indx = 0;
 
 sum = 0.0;
 
  for( tap_indx=0; tap_indx<Num_Numer_Coeff; tap_indx++)
    {
    term = Numer_Coeff[tap_indx] * Input_Buffer[input_read_indx];
    sum += term;
    input_read_indx--;
    if(input_read_indx < 0) input_read_indx = Num_Numer_Coeff-1;
   }

 output_read_indx = Output_Write_Indx;
 for( tap_indx=1; tap_indx<=Num_Denom_Coeff; tap_indx++)
   {
    term = Denom_Coeff[tap_indx] * 
                         Output_Buffer[output_read_indx];
    sum += term;
    output_read_indx--;
    if(output_read_indx < 1) 
                      output_read_indx = Num_Denom_Coeff;
   }

 Output_Write_Indx++;
 if(Output_Write_Indx > Num_Denom_Coeff) 
                                   Output_Write_Indx = 1;
 Output_Buffer[Output_Write_Indx] = sum;

 output_val = sum;
 return(output_val);
}

int UnquantDirectFormIir::GetNumNumerCoeff(void)
{
 return(Num_Numer_Coeff);
}

int UnquantDirectFormIir::GetNumTaps(void)
{
 return(Num_Numer_Coeff);
}


int UnquantDirectFormIir::GetNumDenomCoeff(void)
{
 return(Num_Denom_Coeff);
}