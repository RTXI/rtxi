//
//  File = con_hamm.h
//

#ifndef _CON_HAMM_H_
#define _CON_HAMM_H_

#include "con_resp.h"

class ContHammingMagResp : public ContinWindowResponse
{
 public:

  // constructor

  ContHammingMagResp( std::istream& uin,
                      std::ostream& uout );

};

  

#endif
  