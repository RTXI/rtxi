// 
//  ai.h 
//
//  $DateTime: 2006/07/27 23:51:45 $
//
//  Model specific information:
//
//  Gain range map
//      Range     -> gain value
//
//  NI 622x: 
//      +/- 10V   -> 0
//      +/- 5V    -> 1
//      +/- 1V    -> 4
//      +/- 200mV -> 5
//
//  NI 625x / NI 628x:
//      +/- 10V   -> 1
//      +/- 5V    -> 2
//      +/- 2V    -> 3
//      +/- 1V    -> 4
//      +/- 500mV -> 5
//      +/- 200mV -> 6
//      +/- 100mV -> 7
//
// -------------------------------
//
// Minimum convert period divisor: (20 MHz timebase)
//
//  NI 622x        
//      Single Channel    -> 80
//      Multiple Channels -> 80
//
//  NI 625x        
//      Single Channel    -> 16
//      Multiple Channels -> 20
//
//  NI 628x         
//      Single Channel    -> 30
//      Multiple Channels -> 40
//
// Minimum convert delay divisor -> 3
//

#ifndef ___ai_h___
#define ___ai_h___

#ifndef ___tMSeries_h___
 #include "tMSeries.h"
#endif

void aiClearFifo                (tMSeries* board );
void adcReset                   (tMSeries* board);
void aiReset                    (tMSeries* board);
void aiPersonalize              (tMSeries* board,
                                  tMSeries::tAI_Output_Control::tAI_CONVERT_Output_Select convertOutputSelect);
void aiDisarm                   (tMSeries* board);
void aiClearConfigurationMemory (tMSeries* board);
void aiConfigureChannel         (tMSeries* board,
                                  u16 channel, 
                                  u16 gain, 
                                  tMSeries::tAI_Config_FIFO_Data::tAI_Config_Polarity polarity, 
                                  tMSeries::tAI_Config_FIFO_Data::tAI_Config_Channel_Type channelType,
                                  tBoolean lastChannel);
void aiEnvironmentalize         (tMSeries* board);
void aiSetFifoRequestMode       (tMSeries* board);
void aiHardwareGating           (tMSeries* board);
void aiTrigger                  (tMSeries* board,
                                  tMSeries::tAI_Trigger_Select::tAI_START1_Select startSource,
                                  tMSeries::tAI_Trigger_Select::tAI_START1_Polarity startPolarity,
                                  tMSeries::tAI_Trigger_Select::tAI_START2_Select refSource,
                                  tMSeries::tAI_Trigger_Select::tAI_START2_Polarity refPolarity);
void aiSampleStop               (tMSeries* board, tBoolean multiChannel);
void aiNumberOfSamples          (tMSeries* board, u32 postTriggerSamples, u32 preTriggerSamples, tBoolean continuous);
void aiSampleStart              (tMSeries* board, 
                                  u32 periodDivisor, 
                                  u32 delayDivisor, 
                                  tMSeries::tAI_START_STOP_Select::tAI_START_Select source, 
                                  tMSeries::tAI_START_STOP_Select::tAI_START_Polarity polarity);
void aiConvert                  (tMSeries* board, u32 periodDivisor, u32 delayDivisor, tBoolean externalSampleClock);
void aiArm                      (tMSeries*  board, tBoolean armSI);
void aiStart                    (tMSeries* board);
void aiStartOnDemand            (tMSeries* board);

#endif // ___ai_h___
