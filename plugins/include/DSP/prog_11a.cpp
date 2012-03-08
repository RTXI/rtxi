 //-----------------------------------------------------------------
 //  file = prog_11a.cpp
 //
 //  Program for Ideal FIR Filters
 //
 //-----------------------------------------------------------------
 #include <stdlib.h> 
 #include <iostream.h> 
 #include "misdefs.h"
 #include "typedefs.h" 
 #include "firideal.h"
 #include "fir_dsgn.h"
 #include "fir_resp.h"
 #include "lin_resp.h"
 #ifdef _DEBUG
 ofstream DebugFile("fir_fund.out", ios::out);
 #endif

 ofstream LogFile("fir_fund.log", ios::out);
 ofstream ResponseFile("fir_resp.txt", ios::out);

 main()
 {
  FirIdealFilter *filter_design;
  FirFilterResponse *filter_response;
 
  filter_design = new FirIdealFilter( cin, cout );
  
  #ifdef _DEBUG
  filter_design->DumpCoefficients(&DebugFile);
  #endif
    
  filter_response = new LinearPhaseFirResponse ( filter_design,
                                                 cin,
                                                 cout);
  filter_response->ComputeMagResp();
  filter_response->DumpMagResp();
  return 0;
 }  

