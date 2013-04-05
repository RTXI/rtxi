//
//  calibration.h --
//
//  Defines a method to extract all the data from the MSeries EEPROM
//
//  $DateTime: 2006/07/27 23:51:45 $
//
#ifndef ___scale_h___
#define ___scale_h___

#ifndef ___osiBus_h___
 #include "osiBus.h"
#endif

typedef struct
{
    u32 order;
    f32 c[4]; 
}tScalingCoefficients;


void eepromReadMSeries (iBus *bus, u8 *eepromBuffer, u32 bufferSize);
void serialNumReadMSeries (iBus *bus, u32 *serialNumber);

//
// aiGetScalingCoefficients --
//
// modeIdx
//    0 -> default
//
// intervalIdx
//    0 -> +/- 10V
//    1 -> +/- 5V
//    2 -> +/- 2V
//    3 -> +/- 1V
//    4 -> +/- 500mV
//    5 -> +/- 200mV
//    6 -> +/- 100mV
//
// channel
//    ignored - all channels use the same ADC
//
void aiGetScalingCoefficients (const u8 *eepromMemory, u32 intervalIdx, u32 modeIdx, u32 channel, tScalingCoefficients *scaling);

//
// aoGetScalingCoefficients --
//
// modeIdx
//    ignored - AO does not use the mode constants
//
// intervalIdx
//    0 -> 10V reference
//    1 -> 5V reference
//    2 -> 2V reference
//    3 -> 1V reference
//
// channel
//   dac number
//
void aoGetScalingCoefficients (const u8 *eepromMemory, u32 intervalIdx, u32 modeIdx, u32 channel, tScalingCoefficients *scaling);

void aiPolynomialScaler (const i32 *raw, f32 *scaled, const tScalingCoefficients *scaling);
void aoLinearScaler     (i32 *raw, const f32 *scaled, const tScalingCoefficients *scaling);

#endif // ___scale_h___
