//
// File = fs_util.cpp
//

#include "fs_util.h"
#include <iostream>

void DumpRectangCompon( double *origins,
                        double *slopes, 
                        int num_trans_samps,
                        double x)
{ 
 double rect_comp;
 int n;
 
 for( n=0; n<num_trans_samps; n++)
   {
    rect_comp = origins[n+1] + x * slopes[n+1];
    std::cout << "rect_comp[" << n << "] = " << rect_comp << std::endl;
   }
 return;
}
//================================================================

void pause( logical pause_enabled)
{
 char input_string[20];
 if(pause_enabled)
   {
    std::cout << "enter anything to continue" << std::endl;
    std::cin >> input_string;
   }
 return;
}
