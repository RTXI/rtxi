/*!
   \file osiEnvirons.h
   \brief Defines export symbols
*/

#ifndef ___mhddk_osiEnvirons_h___
#define ___mhddk_osiEnvirons_h___

#ifndef ___osiPlatform_h___
   #include "osiPlatform.h"
#endif


/* kMHDDKExportSymbols directs the build to export symbols modified by
   the kMHDDKExport keyword. kMHDDKNoExportSymbols directs the build to not
   import or export symbols modified by the kMHDDKExport keyword. If
   neither of these are defined, the symbols modified by kMHDDKExport are
   imported. These should be defined only when building the component,
   so clients do not need to trouble themselves with it.
*/
#if defined(kMHDDKExportSymbols)
   #define kMHDDKExport kExport
   #define kMHDDKExportData kExportData
#elif defined(kMHDDKNoExportSymbols)
   #define kMHDDKExport
   #define kMHDDKExportData
#else
   #define kMHDDKExport kImport
   #define kMHDDKExportData kImportData
#endif


#endif // ___mhddk_osiEnvirons_h___

