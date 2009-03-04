#ifndef  __osiTypes_h__
#include "osiTypes.h"
#endif

#ifndef ___nimdbg_tStatus2_h___
#define ___nimdbg_tStatus2_h___

class nMDBG{
   public:
   class tStatus2{
      public:
      i32 tStatus;
      i32 isFatal() {return (tStatus<0);};
      void setCode(int code, int location){ tStatus = code;}
      operator i32*() {return &tStatus;};

      operator i32&() {return tStatus;};

   };
};

#define kPALStatusSoftwareFault	-1
#define NIM_LOCATION			__LINE__
#define NIM_REQUIRE(a,b)		;
#define NIM_CHECK(a,b)		;
#define NIM_TRACE_VAR(a)		;

#define NIM_TRACE_LOAD(a,b,c)   ;
#define NIM_TRACE_LOAD_AT_LEVEL(a,b,c,d)   ;
#define NIM_TRACE_STORE(a,b,c)   ;
#define NIM_TRACE_STORE_AT_LEVEL(a,b,c,d)   ;

#endif
