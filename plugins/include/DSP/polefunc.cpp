//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  File = polefunc.cpp
//
//  Arbitrary All-Pole Filter Response
//

#include <math.h>
#include "misdefs.h"
#include "polefunc.h"

#ifdef _DEBUG
extern ofstream DebugFile;
#endif

//======================================================
//  constructor

AllPoleTransFunc::AllPoleTransFunc( istream& uin,
                                    ostream& uout )
                          :FilterTransFunc()
{
  int k, order;
  double real_part, imag_part;
  uout << "number of poles?" << endl;
  uin >> order;
  Filter_Order = order;
  Prototype_Pole_Locs = new complex[order+1];
  Num_Prototype_Poles = order;
  Prototype_Zero_Locs = new complex[1];
  Num_Prototype_Zeros = 0;
 
  H_Sub_Zero = 1.0;
 
  if(order%2 ==0)
    { // order is even
    uout << "poles must occur in " << order/2
         << " complex conjugate pairs" << endl;
    uout << "each pair will be entered as a real part\n"
         << "and the imaginary part of the pole in the upper half plane\n"
         << "(i.e. sign of imaginary part will always be positive)" << endl;
    for( k=1; k<=(order/2); k++)
      {
      uout << "real part of pole pair " << k << "?" << endl;
      uin >> real_part;
      uout << "pos imag part of pole pair " << k << "?" << endl;
      uin >> imag_part;
      Prototype_Pole_Locs[k] = complex( real_part, imag_part ); 
      Prototype_Pole_Locs[order+1-k] = complex( real_part, -imag_part ); 
      }
    }
  else
    {  // order is odd
    uout << "one pole will be real" << endl;
    uout << "other poles must occur in " << order/2
         << " complex conjugate pairs" << endl;
    uout << "each pair will be entered as a real part\n"
         << "and the imaginary part of the pole in the upper half plane\n"
         << "(i.e. sign of imaginary part will always be positive)" << endl;
    uout << "\nenter value for real pole" << endl;
    uin >> real_part;
    Prototype_Pole_Locs[(order+1)/2] = complex( real_part, 0.0 ); 

    for( k=1; k<=((order-1)/2); k++)
      {
      uout << "real part of pole pair " << k << "?" << endl;
      uin >> real_part;
      uout << "pos imag part of pole pair " << k << "?" << endl;
      uin >> imag_part;
      Prototype_Pole_Locs[k] = complex( real_part, imag_part ); 
      Prototype_Pole_Locs[order+1-k] = complex( real_part, -imag_part ); 
      }
    } 
 return;
};
