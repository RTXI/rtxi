/* 
   osiPlatform.h
*/

#ifndef  ___osiPlatform_h___
#define  ___osiPlatform_h___

// Operating System defines

#define  nOSINT100_kLinux            0
#define  nOSINT100_kMacOSX           0
#define  nOSINT100_kPharLap          0
#define  nOSINT100_kVenturComRTX     0
#define  nOSINT100_kWindows          0

// Processor defines

#define  nOSINT100_kIntel86          0
#define  nOSINT100_kPowerPC          0
   
// Compiler defines

#define  nOSINT100_kApple            0
#define  nOSINT100_kGNU              0
#define  nOSINT100_kMicrosoft        0

// OS, Processor and Compiler Detection

#if (defined(_MSC_VER) && (defined(_M_IX86) || defined(_M_I86)))
   #undef   nOSINT100_kIntel86
   #define  nOSINT100_kIntel86          1
   
   #undef   nOSINT100_kMicrosoft
   #define  nOSINT100_kMicrosoft        _MSC_VER
#elif     ((defined(__GNUG__) || defined(__GNUC__)) && defined(__i386__))
   #undef   nOSINT100_kIntel86
   #define  nOSINT100_kIntel86          1

   #undef   nOSINT100_kGNU
   #define  nOSINT100_kGNU              1
#elif     ((defined(__GNUG__) || defined(__GNUC__)) && defined(__ppc__) && defined(__APPLE__))
   #undef   nOSINT100_kMacOSX
   #define  nOSINT100_kMacOSX          1

   #undef   nOSINT100_kPowerPC
   #define  nOSINT100_kPowerPC         1

   #undef   nOSINT100_kApple
   #define  nOSINT100_kApple           1
#else
   #error   Unknown target;
#endif

#if      nOSINT100_kIntel86
   #if      defined(__linux__)
      #undef   nOSINT100_kLinux
      #define  nOSINT100_kLinux            1
   #elif    defined(PHARLAPOS)
      #undef   nOSINT100_kPharLap
      #define  nOSINT100_kPharLap          1
   #elif    defined(RTXOS)
      #undef   nOSINT100_kVenturComRTX
      #define  nOSINT100_kVenturComRTX     1
   #elif    defined(WINDOWSOS)
      #undef   nOSINT100_kWindows
      #define  nOSINT100_kWindows          1
   #elif    defined(_WIN32)
      #undef   nOSINT100_kWindows
      #define  nOSINT100_kWindows          1
   #else
      #error   Unknown target operating system;
   #endif
#endif

// Define endianness

#if  nOSINT100_kIntel86
   #define  nOSINT100_kBigEndian    0
   #define  nOSINT100_kLittleEndian 1
#elif  nOSINT100_kPowerPC
   #define  nOSINT100_kBigEndian    1
   #define  nOSINT100_kLittleEndian 0
#else
   #error  Unknown processor; 
#endif

// language defines

#if defined(__cplusplus)

   #define  nOSINT100_kC                0
   #define  nOSINT100_kCpp              1
   #define  nOSINT100_cppHeader         extern "C" {
   #define  nOSINT100_cppTrailer        }
   #define  nOSINT100_EXTERNC           extern "C"
   
#else

   #define  nOSINT100_kC                1
   #define  nOSINT100_kCpp              0
   #define  nOSINT100_cppHeader
   #define  nOSINT100_cppTrailer
   #define  nOSINT100_EXTERNC   
   
#endif

// OS-Specific Defines

#if nOSINT100_kLinux
   
   #define  nOSINT100_kImport
   #define  nOSINT100_kExport        __attribute__ ((section (".export")))
   #define  nOSINT100_kImportData
   #define  nOSINT100_kExportData    __attribute__ ((section (".exportData")))
   #define  nOSINT100_kCCall
   #define  nOSINT100_kStdCall
   
#endif

#if nOSINT100_kMacOSX
   
   #define  nOSINT100_kImport
   #define  nOSINT100_kExport        __attribute__ ((section ("__TEXT,__export")))
   #define  nOSINT100_kImportData
   #define  nOSINT100_kExportData    __attribute__ ((section ("__DATA,__export")))
   #define  nOSINT100_kCCall
   #define  nOSINT100_kStdCall

#endif

#if nOSINT100_kPharLap
   
   #define  nOSINT100_kImport           __declspec(dllimport)
   #define  nOSINT100_kExport           __declspec(dllexport)
   #define  nOSINT100_kImportData       __declspec(dllimport)
   #define  nOSINT100_kExportData       __declspec(dllexport)
   #define  nOSINT100_kCCall            __cdecl
   #define  nOSINT100_kStdCall          __stdcall
   
#endif

#if nOSINT100_kVenturComRTX
   
   #define  nOSINT100_kImport           __declspec(dllimport)
   #define  nOSINT100_kExport           __declspec(dllexport)
   #define  nOSINT100_kImportData       __declspec(dllimport)
   #define  nOSINT100_kExportData       __declspec(dllexport)
   #define  nOSINT100_kCCall            __cdecl
   #define  nOSINT100_kStdCall          __stdcall
   
#endif

#if nOSINT100_kWindows

   #define  nOSINT100_kImport           __declspec(dllimport)
   #define  nOSINT100_kExport           __declspec(dllexport)
   #define  nOSINT100_kImportData       __declspec(dllimport)
   #define  nOSINT100_kExportData       __declspec(dllexport)
   #define  nOSINT100_kCCall            __cdecl
   #define  nOSINT100_kStdCall          __stdcall
   
#endif

// Sanity checks --

// Operating Systems
#if !(nOSINT100_kLinux || nOSINT100_kMacOSX || nOSINT100_kPharLap || nOSINT100_kVenturComRTX || nOSINT100_kWindows)
    #error Operating System unknown!
#endif

// Processors
#if !(nOSINT100_kIntel86  || nOSINT100_kPowerPC)
    #error Processor unknown!
#endif

// Compilers
#if !(nOSINT100_kApple || nOSINT100_kGNU || nOSINT100_kMicrosoft)
    #error Compiler unknown!
#endif

// Global definitions

#ifndef nOSINT100_DisableGlobalDefines

#ifndef nOSINT100_DisableGlobalOSDefines
    #define  kLinux           nOSINT100_kLinux
    #define  kMacOSX          nOSINT100_kMacOSX
    #define  kPharLap         nOSINT100_kPharLap
    #define  kkVenturComRTX   nOSINT100_kVenturComRTX
    #define  kWindows         nOSINT100_kWindows
#endif

#ifndef nOSINT100_DisableGlobalProcessorDefines
    #define  kIntel86   nOSINT100_kIntel86
    #define  kPowerPC   nOSINT100_kPowerPC
#endif

#ifndef nOSINT100_DisableGlobalCompilerDefines
    #define  kApple         nOSINT100_kApple
    #define  kGNU           nOSINT100_kGNU
    #define  kMicrosoft     nOSINT100_kMicrosoft
#endif

#ifndef nOSINT100_DisableGlobalEXTERNCDefine
    #define  EXTERNC        nOSINT100_EXTERNC
#endif

#ifndef nOSINT100_DisableGlobalLanguageDefines
    #define  kC                nOSINT100_kC
    #define  kCpp              nOSINT100_kCpp
    #define  cppHeader         nOSINT100_cppHeader
    #define  cppTrailer        nOSINT100_cppTrailer
#endif

#ifndef nOSINT100_DisableGlobalEndiannessDefines
    #define  kBigEndian         nOSINT100_kBigEndian
    #define  kLittleEndian      nOSINT100_kLittleEndian 
#endif

#ifndef nOSINT100_DisableGlobalExportDefines
    #define  kImport           nOSINT100_kImport
    #define  kExport           nOSINT100_kExport
    #define  kImportData       nOSINT100_kImportData
    #define  kExportData       nOSINT100_kExportData
    #define  kCCall            nOSINT100_kCCall
    #define  kStdCall          nOSINT100_kStdCall
#endif

#endif //nOSINT100_DisableGlobalDefines


#endif //___osiPlatform_h___
