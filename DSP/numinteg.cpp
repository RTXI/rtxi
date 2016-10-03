//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  File = numinteg.cpp
//
//  numerical integrator
//

#include <math.h>
#include "numinteg.h"

//======================================================
//  constructor that initializes the integrator
//------------------------------------------------------

NumericInteg::NumericInteg( double delta_t )
{
 Delta_T = delta_t;
 Integ_Mem = 0.0;
 return;
};
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++      


//======================================================
//
//------------------------------------------------------

double NumericInteg::Integrate( double input )
{ 
Integ_Mem += (Delta_T * input);
return(Integ_Mem);
}
