/*
   osiTypes.h
   
   Contains the constants and macros needed for ChipObjects 
   and the iBus implementation.
   
   $DateTime: 2006/07/27 23:51:45 $
   
*/
#ifndef ___osiTypes_h___
#define ___osiTypes_h___

#define kLittleEndian 1

// Note: these different typedef's may be different
// depending on you system.  On a 32 bit processor with a 32 bit
// OS, these will probably not need to be modified.

typedef signed char     i8;
typedef unsigned char   u8;
typedef short            i16;
typedef unsigned short  u16;
typedef signed int      i32;
typedef unsigned int    u32;
typedef float            f32;
typedef double           f64;
typedef char             tText;
typedef char             tChar;
typedef i32               tStatus;

typedef u32               tBoolean;

typedef void *           ptr_t;
typedef unsigned long    ptr_uint_t;

enum { kFalse = 0, kTrue = 1};

//
// Set to match the target endianness of the target platform
//

inline u32 SwitchEndianU32(u32 x)
{
   volatile u32 rval;
   u8  *tmpPtr;

   rval = x;

   tmpPtr = (u8 *)(&rval);
   tmpPtr[0] ^= tmpPtr[3];
   tmpPtr[3] ^= tmpPtr[0];
   tmpPtr[0] ^= tmpPtr[3];

   tmpPtr[1] ^= tmpPtr[2];
   tmpPtr[2] ^= tmpPtr[1];
   tmpPtr[1] ^= tmpPtr[2];

   return rval;
}

inline u16 SwitchEndianU16(u16 x)
{
   volatile u16 rval;
   u8  *tmpPtr;
    
   rval = x;

   tmpPtr = (u8 *)(&rval);
    
   tmpPtr[0] ^= tmpPtr[1];
   tmpPtr[1] ^= tmpPtr[0];
   tmpPtr[0] ^= tmpPtr[1];
    
   return rval;
}

/*
   kLittleEndian=1 or kBigEndian=1 is defined in the 
   OS specific makefile or project
*/

#if kLittleEndian

   #define ReadLittleEndianU32(x) (x)
   #define ReadLittleEndianU16(x) (x)

   #define ReadBigEndianU32(x) (SwitchEndianU32(x))
   #define ReadBigEndianU16(x) (SwitchEndianU16(x))

#elif kBigEndian

   #define ReadLittleEndianU32(x) (SwitchEndianU32(x))
   #define ReadLittleEndianU16(x) (SwitchEndianU16(x))

   #define ReadBigEndianU32(x) (x)
   #define ReadBigEndianU16(x) (x)

#else 

   /*
      In your build project define
         kLittleEndian=1
      or
         kBigEndian=1
      to specify your platform's endianness
   */
   
   #error target endianness not specified
#endif

#define  markAsUnused(type,variable)   {type* _variable = (type*) &variable;}
#define  kStatusOffset                    -50000
#define  kStatusSuccess                   0
#define  kStatusBadWindowType             (-16  + kStatusOffset)

#ifndef NULL
 #define  NULL 0
#endif

#endif // ___osiTypes_h___
