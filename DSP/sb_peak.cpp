//    
//   file = sb_peak.cpp
//     

#include "fs_spec.h"
#include <iostream>

double FindStopbandPeak(  FreqSampFilterSpec *filter,
                          int numPts,
                          double H[])
{
double peak;
int n, nBeg, nEnd, indexOfPeak;

std::cout << "doing case " << filter->GetBandConfig() << std::endl;
switch (filter->GetBandConfig())
  {
   case 1:       /* lowpass */
     nBeg = 2*numPts*(filter->GetN2())/(filter->GetNumTaps());
     nEnd = numPts-1;
    break;
  case 2:       /* highpass */
  case 3:       /* bandpass */
    nBeg = 0;
    nEnd = 2*numPts*filter->GetN1()/filter->GetNumTaps();
    break;
  case 4:       /* bandstop */
    nBeg = 2*numPts*filter->GetN2()/filter->GetNumTaps();
    nEnd = 2*numPts*filter->GetN3()/filter->GetNumTaps();
    break;
  }
std::cout << "nBeg = " << nBeg << std::endl;
std::cout << "nEnd = " << nEnd << std::endl;

peak = -9999.0;
for(n=nBeg; n<nEnd; n++) {
  if(H[n]>peak) {
    peak=H[n];
    indexOfPeak = n;
    }
  }
if(filter->GetBandConfig() == 4) {   /* bandpass has second stopband */
  nBeg = 2*numPts*filter->GetN4()/filter->GetNumTaps();
  nEnd = numPts;
  for(n=nBeg; n<nEnd; n++) {
    if(H[n]>peak) {
      peak=H[n];
      indexOfPeak = n;
      }
    }
  }
return(peak);
}

