/*
   osiBus.h
   
   PCI/PXI interface.
   
   $DateTime: 2006/07/27 23:51:45 $
   
*/
#ifndef  ___osiBus_h___
#define  ___osiBus_h___

#ifndef ___osiTypes_h___
 #include "osiTypes.h"
#endif

/*
   There are two C++ classes declared in osiBus.h:

   iBus -- Think of this as the DAQ board.  acquireBoard(...) returns an iBus.

   tAddressSpace -- This is where you can access registers on the DAQ board.  Since
      PCI/PXI boards can have several BARs, you may need several address spaces
      to access all the registers on the board.

   iBus represents the DAQ board.  tAddressSpace represents an address space
   on the DAQ board.
*/
class iBus;

/*
   Defines an address space within a device (bus)
    
   Since PCI/PXI boards can have several base address ranges, each BAR
   matches an address space.  Then all reads and writes are to the address
   space.  An address space is created from iBus::createAddressSpace(...) 
   and destroyed with iBus::destroyAddressSpace(...).
    
   This implementation of an address space accesses memory mapped boards
   by dereferencing offsets as pointers.  Because the read/write functions are inline
   it is quite fast and is usually compiled away.
*/
class tAddressSpace
{
   private:
      
      void *theSpace;
    
   public:
      
      tAddressSpace(void *ptr): 
         theSpace(ptr) 
      {}
         
      tAddressSpace():
         theSpace(NULL)
      {}
        
      operator u8*() const
      {
         return (u8*)theSpace;
      }

      inline void write8  (const u32 offset, const u32 data);
      inline void write16 (const u32 offset, const u32 data);
      inline void write32( const u32 offset, const u32 data);
      
      inline u8  read8  (const u32 offset);
      inline u16 read16 (const u32 offset);
      inline u32 read32 (const u32 offset);
};

/*
   tBusSpaceReference is another name for tAddressSpace
   The ChipObjects call tAddressSpace tBusSpaceReference
*/
typedef tAddressSpace tBusSpaceReference;


/*
   acquireBoard() finds a PCI/PXI DAQ board which matches the provided arguments.
   
   brdLocation  - VISA like PXI resource string (PXI<bus>::<device/slot>::INSTR)
   devicePCI_ID - Match vendor/device ID (for example, 0x10931870)
*/
iBus* acquireBoard(u32 devBAR0,u32 devBAR1);
//~ iBus* acquireBoard(const u32 devicePCI_ID);

/*
   releaseBoard() releases all OS resources acquired in acquireboard.  Call releaseBoard() once all IO with the
   board has finished.
*/
void releaseBoard(iBus *&busObject);

enum
{  
   kBusAddressPhysical,
   kBusAddressUser,
   kIsPciPxiBus
};

typedef enum 
{
   kPCI_BAR0=0, 
   kPCI_BAR1,
   kPCI_BAR2,
   kPCI_BAR3,
   kPCI_BAR4,
   kPCI_BAR5,
} tBusWindowType;

#define  kBusDeviceHandleInvalid    ((void*)0xffffffffL)
#define  kBusDeviceHandleHost       ((void*)0x00000000L)

/*
   tDMAMemory

   Describes a block of DMA memory.

   OS specific implementation can inherit from tDMAMemory to 
   add OS specific data
*/
class tDMAMemory
{
   public:

      tDMAMemory (void * vAddress, u32 pAddress, u32 size):
         _address (vAddress), 
         _physicalAddress (pAddress), 
         _size (size)
      {}
      
      virtual ~tDMAMemory ();
    
      u32 getSize ()
      {
         return _size;
      }
      
      u32 getPhysicalAddress ()
      {
         return _physicalAddress;
      }
      
      void *getAddress()
      {
         return _address;
      }
    
   private:
      void *_address; 
      u32   _physicalAddress; 
      u32   _size; 
};


/*
   Abstracts OS specific commucation with the device

   Tells the OS how to allow communication with the PCI/PXI DAQ board
   and keeps track of information about the board that may be useful, such as
   the physical address of BAR1, or OS specific data which will be needed to release
   the board when the program has finished.
*/
class iBus
{
   public:
    
    
      /*
         Create an address space to read and write to the DAQ board
         windowType indicate which address space to create
         status optional status
      */
      inline tAddressSpace createAddressSpace(
         const tBusWindowType windowType,
         tStatus *status = NULL);
        
      /*
         Destroy an address space created with createAddressSpace
         addressSpace reference to tAddressSpace object
      */
      inline void destroyAddressSpace(tAddressSpace &addressSpace);
    
      /*
         attribute accessor
            
         Get iBus attributes, such as the physical address of the PCI/PXI board, which is used  
         to initialize the MITE on the DAQ board.
      */
      u32 get(u32 attribute, u32 occurrence, tStatus *status = NULL);
      
      /*
         allocate DMA memory block
      */
      tDMAMemory * allocDMA (u32 size);
        
      /*
         destroy tDMAMemory objects created by allocDMA
    
         Release any OS dependent resources acquired in allocDMA.  Do not call delete directly on the tDMAMemory
         object
      */
      void freeDMA (tDMAMemory *mem);

   private:
       
      inline iBus(
         u32   deviceNumber,
         u32   interruptNumber,
         void* bar0,
         void* bar1 = kBusDeviceHandleInvalid,
         void* bar2 = kBusDeviceHandleInvalid,
         void* bar3 = kBusDeviceHandleInvalid,
         void* bar4 = kBusDeviceHandleInvalid,
         void* bar5 = kBusDeviceHandleInvalid);

      /*
         Create a iBus by calling acquireBoard(), then clean up
         at the end of the program by calling releaseBoard() for each iBus
         that was created with acquireBoard()
      */
    friend iBus* acquireBoard(u32 devBAR0,u32 devBAR1);
      //~ friend iBus* acquireBoard(const u32 devicePCI_ID);
      friend void  releaseBoard(iBus* &busObject);

      inline ~iBus(void);

      /*
         These variables will probably be used by every implementation of a PCI/PXI 
         iBus.  They are OS independent attributes common to all PCI/PXI boards.
      */
      u8* _bar[6];
      u32 _deviceNumber;
      u32 _interruptNumber;
      u32 _physBar[6];
      u32 _isPCI;
      
      /*
         This pointer points to variables which are unique to this OS and OS interface.
         If you port this class to another OS, or interface to the same OS differently, you
         will probably use it to point to different values or structures.
      */
      void *_osSpecific;
};


//
// Inline functions
//

//
// createAddressSpace()/ destroyAddressSpace()
//
// Use createAddressSpace() to create an address space.  The registers for the PCI/PXI
// board are usually in the BAR1 address space, but the MITE is in the BAR0 address
// space.
//
inline tAddressSpace iBus::createAddressSpace(
   const tBusWindowType windowType,
   tStatus *callerStatus)
{
   tStatus status;
   tAddressSpace bar = 0x0;
   
   status = kStatusSuccess;

   switch (windowType) 
   {
      case kPCI_BAR0:
         bar = _bar[0];
         break;

      case kPCI_BAR1:
         bar = _bar[1];
         break;

      case kPCI_BAR2:
         bar = _bar[2];
         break;

      case kPCI_BAR3:
         bar = _bar[3];
         break;

      case kPCI_BAR4:
         bar = _bar[4];
         break;

      case kPCI_BAR5:
         bar = _bar[5];
         break;

      default:
         status = kStatusBadWindowType;
         break;
   }

   if (callerStatus!=NULL) *callerStatus = status;
   return bar;
}


inline void iBus::destroyAddressSpace(tAddressSpace &addressSpace)
{
    // Nothing to do for a iBus
}
/////////////////////////////////////////////////////////////////
// tAddressSpace
// Reads and Writes
// 
// These look more complicated than they should to work around a known compiler
// bug in versions of gcc prior to 3.0.4.  

inline void tAddressSpace::write8(const u32 registerOffset, const u32 data)
{
   volatile u8* p = ((u8*) theSpace) + registerOffset;
   (void)(*p = (u8) data);
}

inline void tAddressSpace::write16( const u32 registerOffset, const u32 data)
{
   volatile u16* p = (u16*) (((u8*) theSpace) + registerOffset);
   (void)(*p = (u16) ReadLittleEndianU16(data));
}

inline void tAddressSpace::write32(const u32 registerOffset, const u32 data)
{
   volatile u32* p = (u32*) (((u8*) theSpace) + registerOffset);
   (void)(*p = ReadLittleEndianU32(data));
}

inline u8 tAddressSpace::read8(const u32 registerOffset)
{
   volatile u8* p = ((u8*) theSpace) + registerOffset;
   u8  data = *p;
   return data;
}

inline u16 tAddressSpace::read16(const u32 registerOffset)
{
   volatile u16* p = (u16*) (((u8*) theSpace) + registerOffset);
   u16  data = *p;
   return ReadLittleEndianU16(data);
}

inline u32 tAddressSpace::read32(const u32 registerOffset)
{
   volatile u32* p = (u32*) (((u8*) theSpace) + registerOffset);
   u32  data = *p;
   return ReadLittleEndianU32(data);
}

// iBus
//
// Constructor/Destructor
// 
// Instead of directly calling the iBus constructor, create a iBus (or iBus)
// using acquireBoard(...).  acquireBoard(...) initializes the 
// private OS specific members too.
inline iBus::iBus(
   u32   deviceNumber,
   u32   interruptNumber,
   void* bar0,
   void* bar1,
   void* bar2,
   void* bar3,
   void* bar4,
   void* bar5):
   _deviceNumber(deviceNumber),
   _interruptNumber(interruptNumber),
   _isPCI(1)
{
   _bar[0] = (u8*) bar0;
   _bar[1] = (u8*) bar1;
   _bar[2] = (u8*) bar2;
   _bar[3] = (u8*) bar3;
   _bar[4] = (u8*) bar4;
   _bar[5] = (u8*) bar5;
}

inline iBus::~iBus(void)
{}



#endif // ___osiBus_h___

