//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  File = polydiv.cpp
//
//

#include <math.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include "complex.h"
#include "polydiv.h"
#ifdef _DEBUG
extern std::ofstream DebugFile;
#endif

void polydiv( complex *dvnd,
              int dvnd_deg,
              complex *dvsr,
              int dvsr_deg,
              complex *quot,
              complex *remndr)
{
  int k,j;

  for(j=0; j<=dvnd_deg; j++)
    {
    remndr[j] = dvnd[j];
    quot[j] = complex(0.0,0.0);
    }
  for(k=dvnd_deg-dvsr_deg; k>=0; k--)
    {
    quot[k] = remndr[dvsr_deg+k]/dvsr[dvsr_deg];
    for(j=dvsr_deg+k-1; j>=k; j--)
      remndr[j] -= quot[k]*dvsr[j-k];
    }
  for(j=dvsr_deg; j<=dvnd_deg; j++) remndr[j] = complex(0.0,0.0);
}
