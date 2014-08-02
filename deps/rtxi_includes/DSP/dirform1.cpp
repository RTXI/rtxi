//
//  File = dirform1.cpp
//

#include <stdlib.h>
#include <fstream>
#include "dirform1.h"
#ifdef _DEBUG
extern std::ofstream DebugFile;
#endif

DirectFormFir::DirectFormFir( int num_taps,
                              double *coeff,
                              logical quan_enab,
                              long coeff_quan_factor,
                              long input_quan_factor)
{
 int n;
 Num_Taps = num_taps;
 Write_Indx = 0;
 Quan_Enab = quan_enab;

 if(quan_enab){
   Quan_In_Buf = new long[num_taps];
   Quan_Coeff = new long[num_taps];
   Input_Quan_Factor = input_quan_factor;
   Long_Out_Quan_Factor = coeff_quan_factor;
   Output_Quan_Factor 
               = double( coeff_quan_factor * input_quan_factor);
   for(n=0; n<num_taps; n++)
     {
      Quan_Coeff[n] = long((coeff_quan_factor * coeff[n]) + 0.5);
      #ifdef _DEBUG
      DebugFile << coeff[n] << " quantized to " 
                << Quan_Coeff[n] << std::endl;
      #endif
      Quan_In_Buf[n] = 0;
     }
 }
 else {
   #ifdef _DEBUG
   DebugFile << "Initializing DirectFormFir unquantized" << std::endl;
   #endif
   Input_Quan_Factor = 32768;
   Long_Out_Quan_Factor = 32768;
   Unquan_In_Buf = new double[num_taps];
   Unquan_Coeff = new double[num_taps];
   for(n=0; n<num_taps; n++)
     {
      Unquan_Coeff[n] = coeff[n];
      Unquan_In_Buf[n] = 0.0;
     }
 }
 
 #ifdef _DEBUG
 DebugFile << "In DirectFormFir" << std::endl;
 #endif
 
 return;
}

double DirectFormFir::ProcessSample( double input_val )
{
 double output_val;
 int read_indx, tap_indx;
 
 if(Quan_Enab) {
   long term, sum;
   Quan_In_Buf[Write_Indx] = long(Input_Quan_Factor * input_val);
 
   read_indx = Write_Indx;
   Write_Indx++;
   if(Write_Indx >= Num_Taps) Write_Indx = 0;
 
   sum = 0;
 
   for( tap_indx=0; tap_indx<Num_Taps; tap_indx++)
     {
      term = Quan_Coeff[tap_indx] * Quan_In_Buf[read_indx];
      sum += term;
    
      read_indx--;
      if(read_indx < 0) read_indx = Num_Taps-1;
     }
   output_val = sum/Output_Quan_Factor;
 }
 else {
   // processing for unquantized implementation
   double term, sum;
   Unquan_In_Buf[Write_Indx] = input_val;
 
   read_indx = Write_Indx;
   Write_Indx++;
   if(Write_Indx >= Num_Taps) Write_Indx = 0;
 
   sum = 0.0;
 
   for( tap_indx=0; tap_indx<Num_Taps; tap_indx++)
     {
      term = Unquan_Coeff[tap_indx] * Unquan_In_Buf[read_indx];
      sum += term;
    
      read_indx--;
      if(read_indx < 0) read_indx = Num_Taps-1;
     }
   output_val = sum;
 }
 
 return(output_val);
}
//-----------------------------------------------------
//
long DirectFormFir::ProcessSample( long input_val )
{
 long output_val;
 int read_indx, tap_indx;
 
 long term, sum;
 Quan_In_Buf[Write_Indx] = input_val;

 read_indx = Write_Indx;
 Write_Indx++;
 if(Write_Indx >= Num_Taps) Write_Indx = 0;

 sum = 0;
 for( tap_indx=0; tap_indx<Num_Taps; tap_indx++)
   {
    term = Quan_Coeff[tap_indx] * Quan_In_Buf[read_indx];
    sum += term;
  
    read_indx--;
    if(read_indx < 0) read_indx = Num_Taps-1;
   }
 output_val = sum/Long_Out_Quan_Factor;

 return(output_val);
}

int DirectFormFir::GetNumTaps(void)
{
 return(Num_Taps);
}