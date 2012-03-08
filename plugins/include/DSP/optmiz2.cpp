//
//  File = optmiz2.cpp
//

#include <math.h>
#include <fstream>
#include "optmiz2.h"
#include "goldsrh2.h"
#include "fs_util.h"
extern std::ofstream LogFile;

void optimize2( FreqSampFilterSpec *filter_spec,
                FreqSampFilterDesign *filter_design,
                FreqSampFilterResponse *filter_resp,
                double y_base_init,
                double tol,
                double tweak_factor,
                double rectComps[])
{
double x1, x2, x3, y3, min_func_val;
double slopes[5], origins[5];
double old_min, x_max;
double y_base;
double rho_min, rho_max;

y_base = y_base_init;
old_min = 9999.0;

for(;;)
  {
  //---------------------------------------------------
  //  do starting point for new steepest descent line
  
  slopes[1] = 1.0;
  slopes[2] = 0.0;
  origins[1] = 0.0;
  origins[2] = y_base;
  rho_min = 0.0;
  rho_max = 1.0;

  x1 = GoldenSearch2( tol,
                      filter_spec,
                      filter_design,
                      filter_resp,
                      rho_min,
                      rho_max,
                      origins,
                      slopes,
                      &min_func_val);
  std::cout << "x1 = " << x1 << std::endl;
  //std::cout << "y3 = " << y3 << std::endl;
  std::cout << "min = " << min_func_val << std::endl;
  pause(TRUE);
  
  /*-------------------------------------*/
  /*  do perturbed point to get          */
  /*    slope for steepest descent line  */
  
  origins[2]=y_base * tweak_factor;
  
  x2 = GoldenSearch2( tol,
                      filter_spec,
                      filter_design,
                      filter_resp,
                      rho_min,
                      rho_max,
                      origins,
                      slopes,
                      &min_func_val);
  
  /*-------------------------------------*/
  /* define line of steepest descent     */
  /*  and find optimal point along line  */
  
  slopes[2] = y_base*(1-tweak_factor)/(x1-x2);
  LogFile << "\n slopes[2] = " << slopes[2] << std::endl;
  origins[2] = y_base - slopes[2] * x1;
  LogFile << "y_base = " << y_base << std::endl;
  LogFile << "x1 = " << x1 << std::endl;
  LogFile << " origins[2] = " << origins[2] << "\n" << std::endl;
  x_max = (1.0 - origins[2])/slopes[2];
  std::cout << "x2 = " << x2 << std::endl;
  //std::cout << "y3 = " << y3 << std::endl;
  std::cout << "min = " << min_func_val << std::endl;
  pause(TRUE);

  x3 = GoldenSearch2( tol,
                      filter_spec,
                      filter_design,
                      filter_resp,
                      rho_min,
                      x_max,
                      origins,
                      slopes,
                      &min_func_val);
  y3=origins[2] + x3 * slopes[2];

  /*---------------------------------------------------------------*/
  /*  if ripple at best point on current line is within specified  */
  /*    tolerance of ripple at best point on previous line,        */
  /*    then stop; otherwise stay in loop and define a new line    */
  /*    starting at the best point on line just completed.         */

  LogFile << "old_min = " << old_min << std::endl;
  LogFile << "min_func_val = " << min_func_val << std::endl;
  //if(fabs(old_min-min_func_val) < tol) break;
  if(fabs(old_min-min_func_val) < 0.1) break;
  old_min = min_func_val;
  y_base = y3;
  std::cout << "x3 = " << x3 << std::endl;
  std::cout << "y3 = " << y3 << std::endl;
  std::cout << "min = " << min_func_val << std::endl;
  pause(TRUE);
  }
rectComps[0] = x3;
rectComps[1] = origins[2] + x3 * slopes[2];
return;
}

