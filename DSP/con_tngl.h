//
//  File = con_tngl.h
//

#ifndef _CON_TNGL_H_
#define _CON_TNGL_H_

#include "con_resp.h"

class ContTriangularMagResp : public ContinWindowResponse
{
 public:

  // constructor

  ContTriangularMagResp( std::istream& uin,
                         std::ostream& uout );

};

  

#endif
  