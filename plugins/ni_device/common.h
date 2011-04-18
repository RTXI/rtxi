//
//  common.h
//
//  $DateTime: 2006/07/27 23:51:45 $
//
#ifndef ___common_h___
#define ___common_h___

#ifndef ___tMSeries_h___
 #include "tMSeries.h"
#endif

void configureTimebase  (tMSeries* board);
void pllReset           (tMSeries* board);
void analogTriggerReset (tMSeries* board);


#endif // ___common_h___

