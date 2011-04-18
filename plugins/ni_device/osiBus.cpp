/*
   osiBus.cpp
   
   osiBus.cpp holds the non-inlined platform
   independent code for the iBus
   
   $DateTime: 2006/07/27 23:51:45 $
   
*/
#include "osiBus.h"

u32 iBus::get(u32 attribute, u32 occurrence, tStatus *status)
{
   if(attribute == kBusAddressPhysical)
   {
      switch(occurrence)
      {
         case    kPCI_BAR0:  return _physBar[0];
         case    kPCI_BAR1:  return _physBar[1];
         case    kPCI_BAR2:  return _physBar[2];
         case    kPCI_BAR3:  return _physBar[3];
         case    kPCI_BAR4:  return _physBar[4];
         case    kPCI_BAR5:  return _physBar[5];
      }
   }
   else if(attribute == kIsPciPxiBus)
   {
      //
      // Set to 1 (PCI/PXI) by default in the constructor
      //
      return _isPCI;
   }
    
   if (status != NULL) *status = -1;
   return (u32)NULL;
}

/*
   tDMAMemory
*/
tDMAMemory::~tDMAMemory()
{
   _address = NULL;  
   _physicalAddress = 0; 
   _size = 0;
}
