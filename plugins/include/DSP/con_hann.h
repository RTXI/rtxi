//
//  File = con_hann.h
//

#ifndef _CON_HANN_H_
#define _CON_HANN_H_

#include "con_resp.h"

class ContHannMagResp : public ContinWindowResponse
{
 public:

  // constructor

  ContHannMagResp( istream& uin,
                   ostream& uout );

};

  

#endif
  