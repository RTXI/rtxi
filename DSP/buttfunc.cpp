//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  File = buttfunc.cpp
//
//  Butterworth Filter Response
//

#include <math.h>
#include "misdefs.h"
#include "buttfunc.h"

#ifdef _DEBUG
extern std::ofstream DebugFile;
#endif

//======================================================
//  constructor

ButterworthTransFunc::ButterworthTransFunc( int order )
                          :FilterTransFunc(order)
{
 double x;
 
 Prototype_Pole_Locs = (complex*) new double[2*(order+1)];
 Num_Prototype_Poles = order;
 Prototype_Zero_Locs = (complex*) new double[2];
 Num_Prototype_Zeros = 0;
 
 H_Sub_Zero = 1.0;
 #ifdef _DEBUG
 DebugFile << "in Butterworth Resp, H_Sub_Zero set to "
           << H_Sub_Zero << std::endl;
 #endif

 for(int k=1; k<=order; k++)
   {
    x = PI * (order + (2*k)-1) / (2*order);
    Prototype_Pole_Locs[k] = complex( cos(x), sin(x) ); 
    #ifdef _DEBUG
    DebugFile << "pole[" << k << "] = "
              << Prototype_Pole_Locs[k] << std::endl;
    #endif
   }
 return;
};
