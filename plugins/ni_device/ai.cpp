// 
//  ai.cpp 
//
//  $DateTime: 2006/07/27 23:51:45 $
//
#include "ai.h"

void aiClearFifo (tMSeries* board )
{
    board->AI_FIFO_Clear.writeRegister(1);
    
    return;
}

void adcReset (tMSeries* board)
{
    board->Static_AI_Control[0].writeRegister (0);
    board->Static_AI_Control[0].writeRegister (1);
    
    board->AI_Command_1.writeAI_CONVERT_Pulse (1);
    board->AI_Command_1.writeAI_CONVERT_Pulse (1);
    board->AI_Command_1.writeAI_CONVERT_Pulse (1);

    return;
}

void aiReset (tMSeries* board)
{
    board->Joint_Reset.writeAI_Reset(1);
    
    board->Joint_Reset.setAI_Configuration_Start (1);
    board->Joint_Reset.flush();
    
    board->Interrupt_A_Enable.setAI_SC_TC_Interrupt_Enable (0);
    board->Interrupt_A_Enable.setAI_START1_Interrupt_Enable (0);
    board->Interrupt_A_Enable.setAI_START2_Interrupt_Enable (0);
    board->Interrupt_A_Enable.setAI_START_Interrupt_Enable (0);
    board->Interrupt_A_Enable.setAI_STOP_Interrupt_Enable (0);
    board->Interrupt_A_Enable.setAI_Error_Interrupt_Enable (0);
    board->Interrupt_A_Enable.setAI_FIFO_Interrupt_Enable (0);
    board->Interrupt_A_Enable.flush ();
    
    board->Interrupt_A_Ack.setAI_SC_TC_Error_Confirm (1);
    board->Interrupt_A_Ack.setAI_SC_TC_Interrupt_Ack (1);
    board->Interrupt_A_Ack.setAI_START1_Interrupt_Ack (1);
    board->Interrupt_A_Ack.setAI_START2_Interrupt_Ack (1);
    board->Interrupt_A_Ack.setAI_START_Interrupt_Ack (1);
    board->Interrupt_A_Ack.setAI_STOP_Interrupt_Ack (1);
    board->Interrupt_A_Ack.setAI_Error_Interrupt_Ack (1);
    board->Interrupt_A_Ack.flush ();
    
    board->AI_Mode_1.setAI_Start_Stop (1);
    board->AI_Mode_1.flush ();
    
    board->AI_Mode_2.writeRegister (0);
    board->AI_Mode_3.writeRegister (0);
    board->AI_Output_Control.writeRegister (0);
    board->AI_Personal.writeRegister (0);
    board->AI_START_STOP_Select.writeRegister (0);
    board->AI_Trigger_Select.writeRegister (0);
    
    board->Joint_Reset.setAI_Configuration_End (1);
    board->Joint_Reset.flush();    
    
    return;
    
}

// NI 622x --
//      tMSeries::tAI_Output_Control::kAI_CONVERT_Output_SelectActive_High
//
// NI 625x --
//      tMSeries::tAI_Output_Control::kAI_CONVERT_Output_SelectActive_Low
//
// NI 628x --
//      tMSeries::tAI_Output_Control::kAI_CONVERT_Output_SelectActive_Low
//

void aiPersonalize (tMSeries* board,
                     tMSeries::tAI_Output_Control::tAI_CONVERT_Output_Select convertOutputSelect)
{
    board->Joint_Reset.setAI_Configuration_Start (1);
    board->Joint_Reset.flush();
    
    board->AI_Output_Control.setAI_CONVERT_Output_Select (convertOutputSelect);
    board->AI_Output_Control.setAI_SCAN_IN_PROG_Output_Select (tMSeries::tAI_Output_Control::kAI_SCAN_IN_PROG_Output_SelectActive_High);
    board->AI_Output_Control.flush ();
    
    board->AI_Personal.setAI_CONVERT_Pulse_Width (tMSeries::tAI_Personal::kAI_CONVERT_Pulse_WidthAbout_1_Clock_Period);
    board->AI_Personal.flush ();
    
    board->Joint_Reset.setAI_Configuration_End (1);
    board->Joint_Reset.flush();
    
    return;
}

void aiDisarm (tMSeries* board)
{
    board->Joint_Reset.writeAI_Reset (1);
    
    return; 
}

void aiClearConfigurationMemory (tMSeries* board)
{
    board->Configuration_Memory_Clear.writeRegister (1);
    
    // Use configuration FIFO
    board->AI_Config_FIFO_Bypass.setAI_Bypass_Config_FIFO (0);
    board->AI_Config_FIFO_Bypass.flush ();
    
    return; 
}

void aiConfigureChannel (tMSeries* board,
                           u16 channel, 
                           u16 gain, 
                           tMSeries::tAI_Config_FIFO_Data::tAI_Config_Polarity polarity, 
                           tMSeries::tAI_Config_FIFO_Data::tAI_Config_Channel_Type channelType,
                           tBoolean lastChannel)
{
    board->AI_Config_FIFO_Data.setAI_Config_Polarity (polarity);
    board->AI_Config_FIFO_Data.setAI_Config_Gain (gain);   
    board->AI_Config_FIFO_Data.setAI_Config_Channel (channel & 0xF);
    board->AI_Config_FIFO_Data.setAI_Config_Bank ((channel & 0x30) >> 4);
    board->AI_Config_FIFO_Data.setAI_Config_Channel_Type (channelType);
    board->AI_Config_FIFO_Data.setAI_Config_Dither (0);
    board->AI_Config_FIFO_Data.setAI_Config_Last_Channel (lastChannel);
    board->AI_Config_FIFO_Data.flush ();   
    
    if (lastChannel)
    {
        board->AI_Command_1.setAI_LOCALMUX_CLK_Pulse (kTrue);
        board->AI_Command_1.flush ();
    }
    
    return;
}

void aiEnvironmentalize (tMSeries* board)
{
    board->Joint_Reset.setAI_Configuration_Start (1);
    board->Joint_Reset.flush();
    
    board->AI_Mode_2.setAI_External_MUX_Present (tMSeries::tAI_Mode_2::kAI_External_MUX_PresentEvery_Convert);
    board->AI_Mode_2.flush ();
    
    board->AI_Output_Control.setAI_EXTMUX_CLK_Output_Select (tMSeries::tAI_Output_Control::kAI_EXTMUX_CLK_Output_SelectGround);
    board->AI_Output_Control.flush ();
    
    board->Joint_Reset.setAI_Configuration_End (1);
    board->Joint_Reset.flush();
    
    return;
}

void aiSetFifoRequestMode (tMSeries* board)
{
    board->AI_Mode_3.setAI_FIFO_Mode (tMSeries::tAI_Mode_3::kAI_FIFO_ModeNot_Empty);
    return;     
}

void aiHardwareGating (tMSeries* board)
{
    board->Joint_Reset.setAI_Configuration_Start (1);
    board->Joint_Reset.flush();
    
    board->AI_Mode_3.setAI_External_Gate_Select (tMSeries::tAI_Mode_3::kAI_External_Gate_SelectDisabled);
    board->AI_Mode_3.flush ();
    
    board->Joint_Reset.setAI_Configuration_End (1);
    board->Joint_Reset.flush();
    
    return;
}

void aiTrigger (tMSeries* board,
                 tMSeries::tAI_Trigger_Select::tAI_START1_Select startSource,
                 tMSeries::tAI_Trigger_Select::tAI_START1_Polarity startPolarity,
                 tMSeries::tAI_Trigger_Select::tAI_START2_Select refSource,
                 tMSeries::tAI_Trigger_Select::tAI_START2_Polarity refPolarity)
{
    board->Joint_Reset.setAI_Configuration_Start (1);
    board->Joint_Reset.flush();
    
    board->AI_Mode_1.setAI_Trigger_Once (1);
    board->AI_Mode_1.flush ();
    
    // Start trigger signal
    board->AI_Trigger_Select.setAI_START1_Select (startSource);
    board->AI_Trigger_Select.setAI_START1_Polarity (startPolarity);
    board->AI_Trigger_Select.setAI_START1_Edge (1);
    board->AI_Trigger_Select.setAI_START1_Sync (1);
    board->AI_Trigger_Select.flush ();
    
    // Reference trigger signal    
    if( refSource == tMSeries::tAI_Trigger_Select::kAI_START2_SelectPulse)
        board->AI_Mode_2.setAI_Pre_Trigger (0);
    else
        board->AI_Mode_2.setAI_Pre_Trigger (1);
    board->AI_Mode_2.flush ();
    
    board->AI_Trigger_Select.setAI_START2_Select (refSource);
    board->AI_Trigger_Select.setAI_START2_Polarity (refPolarity);
    board->AI_Trigger_Select.setAI_START2_Edge (1);
    board->AI_Trigger_Select.setAI_START2_Sync (1);
    board->AI_Trigger_Select.flush ();
    
    board->Joint_Reset.setAI_Configuration_End (1);
    board->Joint_Reset.flush();
    
    return;
}

void aiSampleStop (tMSeries* board, tBoolean multiChannel)
{
    board->Joint_Reset.setAI_Configuration_Start (1);
    board->Joint_Reset.flush();    
    
    board->AI_START_STOP_Select.setAI_STOP_Select (tMSeries::tAI_START_STOP_Select::kAI_STOP_SelectIN);
    board->AI_START_STOP_Select.setAI_STOP_Polarity (tMSeries::tAI_START_STOP_Select::kAI_STOP_PolarityActive_High);
    board->AI_START_STOP_Select.setAI_STOP_Sync (1);
    board->AI_START_STOP_Select.setAI_STOP_Edge (multiChannel);
    board->AI_START_STOP_Select.flush ();
    
    board->Joint_Reset.setAI_Configuration_End (1);
    board->Joint_Reset.flush();
    
    return;
}

void aiNumberOfSamples ( tMSeries* board, 
                          u32 postTriggerSamples,
                          u32 preTriggerSamples, 
                          tBoolean continuous)
{
    
    board->Joint_Reset.setAI_Configuration_Start (1);
    board->Joint_Reset.flush();    
    
    board->AI_Mode_1.setAI_Continuous (continuous);
    board->AI_Mode_1.flush ();
    
    board->AI_SC_Load_A.writeRegister (postTriggerSamples-1);
    

    if (!preTriggerSamples)
    {
        board->AI_SC_Load_B.setRegister (0);
        
        board->AI_Mode_2.setAI_SC_Reload_Mode (tMSeries::tAI_Mode_2::kAI_SC_Reload_ModeNo_Change);
        board->AI_Mode_2.setAI_SC_Initial_Load_Source (tMSeries::tAI_Mode_2::kAI_SC_Initial_Load_SourceLoad_A);
    }
    else
    {
        board->AI_SC_Load_B.setRegister ((preTriggerSamples-1));
        
        board->AI_Mode_2.setAI_SC_Reload_Mode (tMSeries::tAI_Mode_2::kAI_SC_Reload_ModeSwitch);
        board->AI_Mode_2.setAI_SC_Initial_Load_Source (tMSeries::tAI_Mode_2::kAI_SC_Initial_Load_SourceLoad_B);
    }
    board->AI_Mode_2.flush ();
    board->AI_SC_Load_B.flush ();
 
    board->AI_Command_1.setAI_SC_Load (1);
    board->AI_Command_1.flush ();
    
    board->Joint_Reset.setAI_Configuration_End (1);
    board->Joint_Reset.flush();
    
    return;
}

void aiSampleStart (tMSeries* board, 
                     u32 periodDivisor, 
                     u32 delayDivisor, 
                     tMSeries::tAI_START_STOP_Select::tAI_START_Select source, 
                     tMSeries::tAI_START_STOP_Select::tAI_START_Polarity polarity)
{
    board->Joint_Reset.setAI_Configuration_Start (1);
    board->Joint_Reset.flush();
    
    board->AI_START_STOP_Select.setAI_START_Select (source);
    board->AI_START_STOP_Select.setAI_START_Polarity (polarity);
    board->AI_START_STOP_Select.setAI_START_Sync (1);
    board->AI_START_STOP_Select.setAI_START_Edge (1);
    board->AI_START_STOP_Select.flush ();
   
    if (source == tMSeries::tAI_START_STOP_Select::kAI_START_SelectSI_TC)
    {
        board->AI_Mode_1.setAI_SI_Source_Polarity (tMSeries::tAI_Mode_1::kAI_SI_Source_PolarityRising_Edge);
        board->AI_Mode_1.setAI_SI_Source_Select (tMSeries::tAI_Mode_1::kAI_SI_Source_SelectINTIMEBASE1);
        board->AI_Mode_1.flush ();
    
        board->AI_SI_Load_B.writeRegister (delayDivisor-1);
        board->AI_SI_Load_A.writeRegister (periodDivisor-1);

        board->AI_Mode_2.setAI_SI_Initial_Load_Source (tMSeries::tAI_Mode_2::kAI_SI_Initial_Load_SourceLoad_B);        
        board->AI_Mode_2.flush ();

        board->AI_Command_1.writeAI_SI_Load (1);

        board->AI_Mode_2.setAI_SI_Reload_Mode (tMSeries::tAI_Mode_2::kAI_SI_Reload_ModeAlternate_First_Period_Every_SCTC);
        board->AI_Mode_2.setAI_SI_Initial_Load_Source (tMSeries::tAI_Mode_2::kAI_SI_Initial_Load_SourceLoad_A);
        board->AI_Mode_2.flush ();
    }

    board->Joint_Reset.setAI_Configuration_End (1);
    board->Joint_Reset.flush();
    
    return;
}

void aiConvert (tMSeries* board, u32 periodDivisor, u32 delayDivisor, tBoolean externalSampleClock)
{
    board->Joint_Reset.setAI_Configuration_Start (1);
    board->Joint_Reset.flush();
    
    board->AI_Mode_1.setAI_CONVERT_Source_Select (tMSeries::tAI_Mode_1::kAI_CONVERT_Source_SelectSI2TC);
    board->AI_Mode_1.setAI_CONVERT_Source_Polarity (tMSeries::tAI_Mode_1::kAI_CONVERT_Source_PolarityFalling_Edge);
    board->AI_Mode_1.flush ();

    board->AI_Mode_2.setAI_SC_Gate_Enable (0);
    board->AI_Mode_2.setAI_Start_Stop_Gate_Enable (0);
    board->AI_Mode_2.setAI_SI2_Reload_Mode (tMSeries::tAI_Mode_2::kAI_SI2_Reload_ModeAlternate_First_Period_Every_STOP);
    board->AI_Mode_2.flush ();

    if (externalSampleClock)
        board->AI_Mode_3.setAI_SI2_Source_Select (tMSeries::tAI_Mode_3::kAI_SI2_Source_SelectINTIMEBASE1);
    else
        board->AI_Mode_3.setAI_SI2_Source_Select (tMSeries::tAI_Mode_3::kAI_SI2_Source_SelectSame_As_SI);
    board->AI_Mode_3.flush ();
    
    board->AI_SI2_Load_A.writeRegister (delayDivisor);
    board->AI_SI2_Load_B.writeRegister (periodDivisor);
    
    board->AI_Mode_2.setAI_SI2_Initial_Load_Source (tMSeries::tAI_Mode_2::kAI_SI2_Initial_Load_SourceLoad_A);
    board->AI_Mode_2.flush ();
    
    board->AI_Command_1.setAI_SI2_Load (1);
    board->AI_Command_1.flush ();
    
    board->AI_Mode_2.setAI_SI2_Initial_Load_Source (tMSeries::tAI_Mode_2::kAI_SI2_Initial_Load_SourceLoad_B);
    board->AI_Mode_2.flush ();
    
    board->Joint_Reset.setAI_Configuration_End (1);
    board->Joint_Reset.flush();
    
    return;
}

void aiArm (tMSeries*  board, tBoolean armSI)
{
    board->AI_Command_1.setAI_SC_Arm (1);
    board->AI_Command_1.setAI_SI2_Arm (1);    

    if (armSI)
        board->AI_Command_1.setAI_SI_Arm (1);
    
    board->AI_Command_1.flush ();
    
    return;
}

void aiStart (tMSeries* board)
{
    board->AI_Command_2.setAI_START1_Pulse (1);
    board->AI_Command_2.flush ();
    
    return;
}

void aiStartOnDemand (tMSeries* board)
{
    board->AI_Command_2.setAI_START_Pulse (1);
    board->AI_Command_2.flush ();
    
    return;
}
