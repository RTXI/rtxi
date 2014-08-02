//
//  File = fs_resp.cpp
//

#include <math.h>
#include <stdlib.h>
#include "fs_resp.h"
#include "misdefs.h"
extern std::ofstream DebugFile;

FreqSampFilterResponse::FreqSampFilterResponse()
{
}

FreqSampFilterResponse::FreqSampFilterResponse( FreqSampFilterDesign *filter_design,
                                                int num_resp_pts,
                                                int db_scale )
{
 Band_Config = (filter_design->Filter_Spec)->GetBandConfig();
 N1 = filter_design->Filter_Spec->GetN1();
 N2 = filter_design->Filter_Spec->GetN2();
 N3 = filter_design->Filter_Spec->GetN3();
 N4 = filter_design->Filter_Spec->GetN4();
 Num_Resp_Pts = num_resp_pts;
 Db_Scale_Enabled = db_scale;
 Num_Taps = filter_design->GetNumTaps();
 Fir_Type = filter_design->Fir_Type;
 Mag_Resp = new double[Num_Resp_Pts];
 return;
}

FreqSampFilterResponse::~FreqSampFilterResponse()
{
 delete []Mag_Resp;
} 

double FreqSampFilterResponse::GetStopbandPeak()
{
double peak;
int n, nBeg, nEnd, indexOfPeak;

std::cout << "doing case " << Band_Config << std::endl;
switch (Band_Config)
  {
   case 1:       /* lowpass */
     nBeg = 2*Num_Resp_Pts*N2/Num_Taps;
     nEnd = Num_Resp_Pts-1;
    break;
  case 2:       /* highpass */
  case 3:       /* bandpass */
    nBeg = 0;
    nEnd = 2*Num_Resp_Pts*N1/Num_Taps;
    break;
  case 4:       /* bandstop */
    nBeg = 2*Num_Resp_Pts*N2/Num_Taps;
    nEnd = 2*Num_Resp_Pts*N3/Num_Taps;
    break;
  }
std::cout << "nBeg = " << nBeg << std::endl;
std::cout << "nEnd = " << nEnd << std::endl;

peak = -9999.0;
for(n=nBeg; n<nEnd; n++) {
  if(Mag_Resp[n]>peak) {
    peak=Mag_Resp[n];
    indexOfPeak = n;
    }
  }
if(Band_Config == 4) {   /* bandpass has second stopband */
  nBeg = 2*Num_Resp_Pts*N4/Num_Taps;
  nEnd = Num_Resp_Pts;
  for(n=nBeg; n<nEnd; n++) {
    if(Mag_Resp[n]>peak) {
      peak=Mag_Resp[n];
      indexOfPeak = n;
      }
    }
  }
return(peak);
}


//=====================================================================
void FreqSampFilterResponse::ComputeMagResp(
                                  FreqSampFilterDesign *filter_design,
                                  int db_scale)
{
int index, L, n;
double lambda, work;

double* coeff = filter_design->GetCoefficients();
std::cout << "in fs_rsp, coeff = " << (void*)coeff << std::endl; 
std::cout << "Num_Taps = " << Num_Taps << std::endl;
std::cout << "Num_Resp_Pts = " << Num_Resp_Pts << std::endl;

for( L=0; L<Num_Resp_Pts; L++)
  {
  lambda = L * PI / (double) Num_Resp_Pts; 
  switch (Fir_Type) {
    case 1:     /* symmetric and odd */
      work = coeff[(Num_Taps-1)/2];
      for( n=1; n<=((Num_Taps-1)/2); n++) {
        index = (Num_Taps-1)/2 - n;
        work = work + 2.0 * coeff[index] * cos(n*lambda);
        }
      break;
    case 2:     /* symmetric and even */
      work = 0.0;
      for( n=1; n<=(Num_Taps/2); n++) {
        index = Num_Taps/2-n;
        work = work + 2.0 * coeff[index] * cos((n-0.5)*lambda);
        }
      break;
    case 3:     /* antisymmetric and odd */
      work = 0.0;
      for( n=1; n<=((Num_Taps-1)/2); n++) {
        index = (Num_Taps-1)/2 - n;
        work = work + 2.0 * coeff[index] * sin(n*lambda);
        }
      break;
    case 4:     /* symmetric and even */
      work = 0.0;
      for( n=1; n<=(Num_Taps/2); n++) {
        index = Num_Taps/2-n;
        work = work + 2.0 * coeff[index] * sin((n-0.5)*lambda);
        }
      break;
    }
      
  if(db_scale)
    {Mag_Resp[L] = 20.0 * log10(fabs(work));}
  else
    {Mag_Resp[L] = fabs(work);}
    
  }
return;
}

void FreqSampFilterResponse::NormalizeResponse( int db_scale)
{
 int n;
 double biggest;
 
 if(db_scale)
   {
    biggest = -100.0; 
    
    for( n=0; n < Num_Resp_Pts; n++)
      {if(Mag_Resp[n]>biggest) biggest = Mag_Resp[n];}
    for( n=0; n < Num_Resp_Pts; n++)
      {Mag_Resp[n] = Mag_Resp[n] - biggest;}
   }
 else
   {
    biggest = 0.0;
    
    for( n=0; n < Num_Resp_Pts; n++)
      {if(Mag_Resp[n]>biggest) biggest = Mag_Resp[n];}
    for( n=0; n < Num_Resp_Pts; n++)
      {Mag_Resp[n] = Mag_Resp[n] / biggest;}
   }
 return;
}

double* FreqSampFilterResponse::GetMagResp( void)
{
 return(Mag_Resp);
}

void FreqSampFilterResponse::DumpMagResp( ofstream* output_stream)
{
 int n;
 std::cout << "in FreqSampFilterResponse::DumpMagResp" << std::endl;
 for( n=0; n < Num_Resp_Pts; n++)
   {
    (*output_stream) << n << ", " << Mag_Resp[n] << std::endl;
   }
 return;
}
