//
//  File = con_rect.h
//

#ifndef _CON_RECT_H_
#define _CON_RECT_H_

#include "con_resp.h"

class ContRectangularMagResp : public ContinWindowResponse
{
 public:

  // constructor

  ContRectangularMagResp( std::istream& uin,
                          std::ostream& uout );

};

  

#endif
  