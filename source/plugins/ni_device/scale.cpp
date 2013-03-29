//
//  scale.cpp --
//
//  Defines a method to extract all the data from the MSeries EEPROM
//
//  $DateTime: 2006/07/27 23:51:45 $
//
#include "scale.h"
#include "osiBus.h"

// private type definitions

typedef union {
    u8  b[4];
    f32 f;
}tEepromF32;

typedef struct
{
    u32 order;
    f32 c[4];
}tMode;

typedef struct
{
    f32 gain;
    f32 offset;
}tInterval; 


// private eeprom map constants

    const u32 kCalibrationAreaOffset = 24;

    const u32 kHeaderSize = 16;

    const u32 kAiModeBlockStart = kHeaderSize;   
    const u32 kAiModeBlockSize = 17;
    const u32 kAiMaxNumberOfModes = 4;
    const u32 kAiModeBlockEnd = kAiModeBlockStart + kAiModeBlockSize*kAiMaxNumberOfModes;

    const u32 kAiIntervalBlockStart = kAiModeBlockEnd;    
    const u32 kAiIntervalBlockSize = 8;
    const u32 kAiMaxNumberOfIntervals = 7;
    const u32 kAiIntervalBlockEnd = kAiIntervalBlockStart + kAiIntervalBlockSize*kAiMaxNumberOfIntervals;

    const u32 kAoModeBlockStart = kAiIntervalBlockEnd;
    const u32 kAoModeBlockSize = 17;
    const u32 kAoMaxNumberOfModes = 1;
    const u32 kAoModeBlockEnd = kAoModeBlockStart + kAoModeBlockSize*kAoMaxNumberOfModes;

    const u32 kAoIntervalBlockStart = kAoModeBlockEnd;  
    const u32 kAoMaxNumberOfIntervals = 4;
    const u32 kAoIntervalBlockSize = 8;
    const u32 kAoIntervalBlockEnd = kAoIntervalBlockStart + kAoIntervalBlockSize*kAoMaxNumberOfIntervals;

    const u32 kAiChannelBlockSize = kAiModeBlockSize*kAiMaxNumberOfModes + kAiIntervalBlockSize*kAiMaxNumberOfIntervals;
    const u32 kAiMaxNumberOfChannels = 1;
    
    const u32 kAoChannelBlockSize = kAoModeBlockSize*kAoMaxNumberOfModes + kAoIntervalBlockSize*kAoMaxNumberOfIntervals;
    const u32 kAoMaxNumberOfChannels = 4;
    

f32 getF32FromEeprom (const u8 *eepromMemory, u32 offset)
{
    tEepromF32 value;
   
#if kLittleEndian
   
    value.b[3] = eepromMemory[offset++];
    value.b[2] = eepromMemory[offset++];
    value.b[1] = eepromMemory[offset++];
    value.b[0] = eepromMemory[offset++];
   
#elif kBigEndian
   
    value.b[0] = eepromMemory[offset++];
    value.b[1] = eepromMemory[offset++];
    value.b[2] = eepromMemory[offset++];
    value.b[3] = eepromMemory[offset++];
   
#endif
   
    return value.f;
}

u32 getCalibrationAreaOffset (const u8 *eepromMemory)
{
    return (eepromMemory[kCalibrationAreaOffset] << 8) | eepromMemory[kCalibrationAreaOffset+1];
}

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
void aoGetScalingCoefficients (const u8 *eepromMemory, u32 intervalIdx, u32 modeIdx, u32 channel, tScalingCoefficients *scaling)
{

    
    u32 calOffset = 0;
    
    calOffset = getCalibrationAreaOffset (eepromMemory); 
    
    tInterval interval; 
    
    u32 intervalOffset = calOffset + kAoIntervalBlockStart + (intervalIdx*kAoIntervalBlockSize) + (channel*kAoChannelBlockSize);
    
    interval.gain   = getF32FromEeprom (eepromMemory, intervalOffset);
    intervalOffset += 4;
    interval.offset = getF32FromEeprom (eepromMemory, intervalOffset);

    scaling->order = 1; 
    scaling->c[0] = interval.offset;
    scaling->c[1] = interval.gain;

    return; 
}

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
void aiGetScalingCoefficients (const u8 *eepromMemory, u32 intervalIdx, u32 modeIdx, u32 channel, tScalingCoefficients *scaling)
{
    u32 calOffset = 0;
    
    calOffset = getCalibrationAreaOffset (eepromMemory);
    
    u32 modeOffset = calOffset + kAiModeBlockStart + (modeIdx*kAiModeBlockSize);
    
    tMode mode;
    
    mode.order = eepromMemory[modeOffset++];
    mode.c[0] = getF32FromEeprom (eepromMemory, modeOffset);
    modeOffset += 4;
    mode.c[1] = getF32FromEeprom (eepromMemory, modeOffset);
    modeOffset += 4;
    mode.c[2] = getF32FromEeprom (eepromMemory, modeOffset);
    modeOffset += 4;
    mode.c[3] = getF32FromEeprom (eepromMemory, modeOffset);
    
    tInterval interval; 
    
    u32 intervalOffset = calOffset + kAiIntervalBlockStart + (intervalIdx*kAiIntervalBlockSize);
    
    interval.gain = getF32FromEeprom (eepromMemory, intervalOffset);
    intervalOffset += 4;
    interval.offset = getF32FromEeprom (eepromMemory, intervalOffset);

    scaling->order = mode.order;
    
    for (u32 i=0; i <= mode.order; i++)
    {
        scaling->c[i] = mode.c[i] * interval.gain;
        if (i == 0)
            scaling->c[i] = scaling->c[i] + interval.offset;
    }
    
    return; 
}

void aiPolynomialScaler (const i32 *raw, f32 *scaled, const tScalingCoefficients *scaling)
{

    *scaled = scaling->c[scaling->order];

    for( i32 j = scaling->order-1 ; j >= 0 ; j--)
    {
        (*scaled) *= (f32)*raw;
        (*scaled) += scaling->c[j];
    } 
    
    return; 
}

void aoLinearScaler (i32 *raw, const f32 *scaled, const tScalingCoefficients *scaling)
{
    *raw = (i32)((*scaled) * scaling->c[1] + scaling->c[0]);
    
    return;
}

//
// Generic eeprom read
//
void readFromEeprom(iBus *bus, u32 offset, u8 *eepromBuffer, u32 bufferSize)
{
    u32 _iowcr1Initial;
    u32 _iowbsr1Initial;
    u32 _iodwbsrInitial; 
    u32 _bar1Value;
    
    tAddressSpace  bar0;    
    tAddressSpace  bar1;    
    
    bar0 = bus->createAddressSpace(kPCI_BAR0);
    bar1 = bus->createAddressSpace(kPCI_BAR1);
    
    // ---- Open EEPROM 
    
    _iodwbsrInitial = bar0.read32 (0xC0);
    bar0.write32 (0xC0, 0x00);

    _iowbsr1Initial = bar0.read32 (0xC4);
    _bar1Value      = bar0.read32 (0x314);

    bar0.write32 (0xC4, (0x0000008B | _bar1Value));

    _iowcr1Initial = bar0.read32(0xF4);

    bar0.write32 (0xF4, 0x00000001 | _iowcr1Initial);
    bar0.write32 (0x30, 0xF);
        
    // ---- Read EEPROM
    
    for(u32 i = 0; i < bufferSize; ++i)
    {
        eepromBuffer[i] = bar1.read8(i + offset);
    }
    
    // ---- Close EEPROM
    
    bar0.write32 (0xC4, _iowbsr1Initial);
    bar0.write32 (0xC0, _iodwbsrInitial);
    bar0.write32 (0xF4, _iowcr1Initial);
    bar0.write32 (0x30, 0x00);
    
    bus->destroyAddressSpace(bar0);
    bus->destroyAddressSpace(bar1);
}

//
// Read calibration info from eeprom
//
void eepromReadMSeries(iBus *bus, u8 *eepromBuffer, u32 bufferSize)
{
    const u32 kStartCalEEPROM = 1024;
    const u32 kEndCalEEPROM = kStartCalEEPROM + 1024;
   
    readFromEeprom(bus, kStartCalEEPROM, eepromBuffer, bufferSize);
}

//
// Read serial number from eeprom
//
void serialNumReadMSeries(iBus *bus, u32 *serialNumber)
{
    readFromEeprom(bus, 4, (u8 *)serialNumber, 4);
   
    //
    // Serial number is returned as big-endian U32
    //
    *serialNumber = ReadBigEndianU32(*serialNumber);
}

