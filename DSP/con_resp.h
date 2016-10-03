//
//  File = con_resp.h
//

#ifndef _CON_RESP_H_
#define _CON_RESP_H_

#include <fstream>
#include "typedefs.h"

class ContinWindowResponse
{
 public:

  // constructor

  ContinWindowResponse( std::istream& uin,
                        std::ostream& uout );
 
 protected:
 
  int Num_Resp_Pts;
  logical Db_Scale_Enab;
  int Window_Shape;
  std::ofstream *Response_File;

};

  

#endif
  