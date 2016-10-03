//
//  File = con_resp.cpp
//

#include <stdlib.h>
#include "misdefs.h"
#include "con_resp.h"

ContinWindowResponse::ContinWindowResponse( istream& uin,
                                            ostream& uout)
{
  logical default_file_ok;
  
  uout << "shape of window?\n  "
       << "0 = Quit\n  "
       << _RECTANGULAR << " = rectangular\n  "
       << _TRIANGULAR << " = triangular\n  "
       << _HAMMING << " = Hamming\n  "
       << _HANN << " = Hann (hanning, vonHann)\n  "
       << _DOLPH_CHEBY << " = Dolph-Chebyshev\n  "
       << std::endl;
  uin >> Window_Shape;
  
  if( Window_Shape < _RECTANGULAR)
    { exit(0);}
    
  uout << "number of points in plot?" << std::endl;
  uin >> Num_Resp_Pts; 
  uout << "scaling?\n"
       << "  0 = linear, 1 = dB"  << std::endl;
  uin >> Db_Scale_Enab;
  
  if( Db_Scale_Enab != 0) Db_Scale_Enab = 1;
  
  uout << "default name for magnitude response output\n"
       << "file is cwinresp.txt\n\n"
       << "is this okay?"
       << "  0 = NO, 1 = YES"
       << std::endl;
  uin >> default_file_ok;
  
  if( default_file_ok)
    {
     Response_File = new ofstream("cwinresp.txt", ios::out);
    }
  else
    {
     char *file_name;
     file_name = new char[31];
     
     uout << "enter complete name for output file"
          << std::endl;
     uin >> file_name;
     Response_File = new ofstream(file_name, ios::out);
     delete []file_name;
    } 
     

}