// 
//  ao.h 
//
//  $DateTime: 2006/07/27 23:51:45 $
//
#ifndef ___ao_h___
#define ___ao_h___

#ifndef ___tMSeries_h___
 #include "tMSeries.h"
#endif

void aoReset (tMSeries *board);
void aoClearFifo (tMSeries *board);
void aoConfigureDAC (tMSeries *board, 
                     u32 dac, 
                     u32 waveformOrder, 
                     tMSeries::tAO_Config_Bank::tAO_DAC_Polarity polarity,
                     tMSeries::tAO_Config_Bank::tAO_Update_Mode mode);
void aoPersonalize (tMSeries *board);
void aoResetWaveformChannels (tMSeries *board);
void aoChannelSelect (tMSeries *board, u32 numberOfChannels);
void aoTrigger (tMSeries *board, 
                tMSeries::tAO_Trigger_Select::tAO_START1_Select source,
                tMSeries::tAO_Trigger_Select::tAO_START1_Polarity polarity);
void aoCount (tMSeries *board, u32 numberOfSamples, u32 numberOfBuffers, tBoolean continuous);
void aoUpdate (tMSeries *board, 
                tMSeries::tAO_Mode_1::tAO_UPDATE_Source_Select source, 
                tMSeries::tAO_Mode_1::tAO_UPDATE_Source_Polarity polarity, 
                u32 periodDivisor);
void aoFifoMode (tMSeries *board, tBoolean fifoRetransmit);
void aoStop (tMSeries *board);
void aoArm (tMSeries *board);
void aoStart (tMSeries *board);
void aoDisarm (tMSeries *board);

#endif // ___ao_h___
