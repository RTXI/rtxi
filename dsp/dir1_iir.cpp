//
//  File = dir1_iir.cpp
//

#include <stdlib.h>
#include <fstream>
#include "dir1_iir.h"
#ifdef _DEBUG
extern std::ofstream DebugFile;
#endif

DirectFormIir::DirectFormIir( int num_numer_coeff,
                              int num_denom_coeff,
                              double *numer_coeff,
                              double *denom_coeff,
                              long coeff_quan_factor,
                              long input_quan_factor)
{
 int n;
 Num_Numer_Coeff = num_numer_coeff;
 Num_Denom_Coeff = num_denom_coeff;
 Input_Buffer = new long[num_numer_coeff];
 Output_Buffer = new long[num_denom_coeff+1];
 Quan_Numer_Coeff = new long[num_numer_coeff];
 Quan_Denom_Coeff = new long[num_denom_coeff+1];
 Input_Write_Indx = 0;
 Output_Write_Indx = 1;
 Input_Quan_Factor = input_quan_factor;
 Coeff_Quan_Factor = coeff_quan_factor;
 Output_Quan_Factor = double( coeff_quan_factor * input_quan_factor);
 
 #ifdef _DEBUG
 DebugFile << "In DirectFormIir" << std::endl;
 #endif
 
 for(n=0; n<num_numer_coeff; n++)
   {
    Quan_Numer_Coeff[n] = long((coeff_quan_factor * numer_coeff[n]) + 0.5);
    #ifdef _DEBUG
    DebugFile << numer_coeff[n] << " quantized to " 
              << Quan_Numer_Coeff[n] << std::endl;
    #endif
    Input_Buffer[n] = 0;
   }
 for(n=1; n<=num_denom_coeff; n++)
   {
    Quan_Denom_Coeff[n] = long((coeff_quan_factor * denom_coeff[n]) + 0.5);
    #ifdef _DEBUG
    DebugFile << denom_coeff[n] << " quantized to " 
              << Quan_Denom_Coeff[n] << std::endl;
    #endif
    Output_Buffer[n] = 0;
   }
 return;
}

long DirectFormIir::ProcessSample( long input_val )
{
 return(input_val);
}
double DirectFormIir::ProcessSample( double input_val )
{
 double output_val;
 long term, sum;
 int input_read_indx, output_read_indx, tap_indx;
 
 Input_Buffer[Input_Write_Indx] = 
                    long(Input_Quan_Factor * input_val);
 input_read_indx = Input_Write_Indx;
 Input_Write_Indx++;
 if(Input_Write_Indx >= Num_Numer_Coeff) 
                                   Input_Write_Indx = 0;
 
 sum = 0;
 
 for( tap_indx=0; tap_indx<Num_Numer_Coeff; tap_indx++)
   {
    term = Quan_Numer_Coeff[tap_indx] * 
                   Input_Buffer[input_read_indx];
    sum += term;
    input_read_indx--;
    if(input_read_indx < 0) input_read_indx = Num_Numer_Coeff-1;
   }

 output_read_indx = Output_Write_Indx;
 for( tap_indx=1; tap_indx<=Num_Denom_Coeff; tap_indx++)
   {
    term = Quan_Denom_Coeff[tap_indx] * 
                         Output_Buffer[output_read_indx];
    sum += term;
    output_read_indx--;
    if(output_read_indx < 1) 
                      output_read_indx = Num_Denom_Coeff;
   }

 Output_Write_Indx++;
 if(Output_Write_Indx > Num_Denom_Coeff) 
                                   Output_Write_Indx = 1;
 Output_Buffer[Output_Write_Indx] = sum/Coeff_Quan_Factor;

 output_val = double(sum)/Output_Quan_Factor;
 return(output_val);
}

int DirectFormIir::GetNumNumerCoeff(void)
{
 return(Num_Numer_Coeff);
}

int DirectFormIir::GetNumTaps(void)
{
 return(Num_Numer_Coeff);
}


int DirectFormIir::GetNumDenomCoeff(void)
{
 return(Num_Denom_Coeff);
}