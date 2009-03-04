// 
//  ao.cpp 
//
//  $DateTime: 2006/07/27 23:51:45 $
//
#include "ao.h"

void aoReset (tMSeries *board)
{
    board->Joint_Reset.setAO_Reset (1);
    board->Joint_Reset.flush ();
    
    board->Joint_Reset.setAO_Configuration_Start (1);
    board->Joint_Reset.flush ();
    
    board->AO_Command_1.setAO_Disarm (1);
    board->AO_Command_1.flush ();
    
    board->Interrupt_B_Enable.setAO_BC_TC_Interrupt_Enable (0);
    board->Interrupt_B_Enable.setAO_START1_Interrupt_Enable (0);
    board->Interrupt_B_Enable.setAO_UPDATE_Interrupt_Enable (0);
    board->Interrupt_B_Enable.setAO_START_Interrupt_Enable (0);
    board->Interrupt_B_Enable.setAO_STOP_Interrupt_Enable (0);
    board->Interrupt_B_Enable.setAO_Error_Interrupt_Enable (0);
    board->Interrupt_B_Enable.setAO_UC_TC_Interrupt_Enable (0);
    board->Interrupt_B_Enable.setAO_FIFO_Interrupt_Enable (0);
    board->Interrupt_B_Enable.flush ();
    
    board->AO_Personal.setAO_BC_Source_Select (tMSeries::tAO_Personal::kAO_BC_Source_SelectUC_TC);
    board->AO_Personal.flush ();
    
    board->Interrupt_B_Ack.setAO_BC_TC_Trigger_Error_Confirm (1);
    board->Interrupt_B_Ack.setAO_BC_TC_Error_Confirm (1);
    board->Interrupt_B_Ack.setAO_UC_TC_Interrupt_Ack (1);
    board->Interrupt_B_Ack.setAO_BC_TC_Interrupt_Ack (1);
    board->Interrupt_B_Ack.setAO_START1_Interrupt_Ack (1);
    board->Interrupt_B_Ack.setAO_UPDATE_Interrupt_Ack (1);
    board->Interrupt_B_Ack.setAO_START_Interrupt_Ack (1);
    board->Interrupt_B_Ack.setAO_STOP_Interrupt_Ack (1);
    board->Interrupt_B_Ack.setAO_Error_Interrupt_Ack (1);
    board->Interrupt_B_Ack.flush ();
    
    board->Joint_Reset.setAO_Configuration_End (1);
    board->Joint_Reset.flush ();
    
    return;
}

void aoClearFifo (tMSeries *board)
{
    board->AO_FIFO_Clear.writeRegister (1);
    return; 
}

void aoConfigureDAC (tMSeries *board, 
                u32 dac, 
                u32 waveformOrder, 
                tMSeries::tAO_Config_Bank::tAO_DAC_Polarity polarity,
                tMSeries::tAO_Config_Bank::tAO_Update_Mode mode)
{
    
    if (dac > tMSeries::kAO_Config_BankArraySize-1)
        return;
    
    board->AO_Config_Bank[dac].setAO_DAC_Offset_Select (0);    // aognd offset
    board->AO_Config_Bank[dac].setAO_DAC_Reference_Select (0); // 10V Internal reference
    board->AO_Config_Bank[dac].setAO_Update_Mode (mode);    
    board->AO_Config_Bank[dac].setAO_DAC_Polarity (polarity);    
    board->AO_Config_Bank[dac].flush ();    
    
    board->Static_AI_Control[dac+4].writeRegister (0); //reference attenuation off
    
    board->AO_Waveform_Order[dac].writeRegister (waveformOrder);
    
    return;
}

void aoPersonalize (tMSeries *board)
{
    board->Joint_Reset.setAO_Configuration_Start (1);
    board->Joint_Reset.flush ();
    
    board->AO_Personal.setAO_UPDATE_Pulse_Timebase (tMSeries::tAO_Personal::kAO_UPDATE_Pulse_TimebaseSelect_By_PulseWidth);
    board->AO_Personal.setAO_UPDATE_Pulse_Width (tMSeries::tAO_Personal::kAO_UPDATE_Pulse_WidthAbout_3_TIMEBASE_Periods);
    board->AO_Personal.setAO_TMRDACWR_Pulse_Width (tMSeries::tAO_Personal::kAO_TMRDACWR_Pulse_WidthAbout_2_TIMEBASE_Periods);
    board->AO_Personal.setAO_Number_Of_DAC_Packages (tMSeries::tAO_Personal::kAO_Number_Of_DAC_PackagesSingle_DAC_mode);
    board->AO_Personal.flush ();
    
    board->AO_Output_Control.setAO_UPDATE_Output_Select (tMSeries::tAO_Output_Control::kAO_UPDATE_Output_SelectHigh_Z);
    board->AO_Output_Control.flush ();
    
    board->AO_Mode_3.setAO_Last_Gate_Disable (1);
    board->AO_Mode_3.flush ();
    
    board->Joint_Reset.setAO_Configuration_End (1);
    board->Joint_Reset.flush ();
    
    return;
}

void aoResetWaveformChannels (tMSeries *board)
{
    
    u32 i; 
    
    for (i=0; i < tMSeries::kAO_Config_BankArraySize; i++)
    {
        board->AO_Config_Bank[i].writeAO_Update_Mode (tMSeries::tAO_Config_Bank::kAO_Update_ModeImmediate);
    }
    
    for (i=0; i < tMSeries::kAO_Waveform_OrderArraySize; i++)
    {
        board->AO_Waveform_Order[i].writeRegister (0xF);
    }
    
    return;
}

void aoChannelSelect (tMSeries *board, u32 numberOfChannels)
{
    board->Joint_Reset.setAO_Configuration_Start (1);
    board->Joint_Reset.flush ();

    numberOfChannels--;
    
    if(!numberOfChannels)
    {
        //single channel
        board->AO_Mode_1.setAO_Multiple_Channels (0);
        board->AO_Output_Control.setAO_Number_Of_Channels (0);
    }
    else
    {
        // multiple channels
        board->AO_Mode_1.setAO_Multiple_Channels (1);
        board->AO_Output_Control.setAO_Number_Of_Channels (numberOfChannels);
    }
    
    board->AO_Mode_1.flush ();
    board->AO_Output_Control.flush ();
    
    board->Joint_Reset.setAO_Configuration_End (1);
    board->Joint_Reset.flush ();
    
    return;     
}

void aoTrigger (tMSeries *board, 
                tMSeries::tAO_Trigger_Select::tAO_START1_Select source,
                tMSeries::tAO_Trigger_Select::tAO_START1_Polarity polarity)
{
    board->Joint_Reset.setAO_Configuration_Start (1);
    board->Joint_Reset.flush ();   

    board->AO_Trigger_Select.setAO_START1_Select (source);
    board->AO_Trigger_Select.setAO_START1_Polarity (polarity);
    board->AO_Trigger_Select.setAO_START1_Edge (1);
    board->AO_Trigger_Select.setAO_START1_Sync (1);
    board->AO_Trigger_Select.flush ();
    
    board->AO_Mode_3.setAO_Trigger_Length (tMSeries::tAO_Mode_3::kAO_Trigger_LengthDA_START1);
    board->AO_Mode_3.flush ();
    
    board->Joint_Reset.setAO_Configuration_End (1);
    board->Joint_Reset.flush (); 
    
    return; 
}

void aoCount (tMSeries *board, u32 numberOfSamples, u32 numberOfBuffers, tBoolean continuous)
{
    board->Joint_Reset.setAO_Configuration_Start (1);
    board->Joint_Reset.flush ();   
    
    tMSeries::tAO_Mode_1::tAO_Continuous continuousMode; 
    
    continuousMode = continuous ? tMSeries::tAO_Mode_1::kAO_ContinuousIgnore_BC_TC : tMSeries::tAO_Mode_1::kAO_ContinuousStop_On_BC_TC;
    
    board->AO_Mode_1.setAO_Continuous (continuousMode);
    board->AO_Mode_1.setAO_Trigger_Once (continuous? 0:1);
    board->AO_Mode_1.flush ();
    
    board->AO_Mode_2.setAO_BC_Initial_Load_Source (tMSeries::tAO_Mode_2::kAO_BC_Initial_Load_SourceReg_A);
    board->AO_Mode_2.setAO_UC_Initial_Load_Source (tMSeries::tAO_Mode_2::kAO_UC_Initial_Load_SourceReg_A);
    board->AO_Mode_2.flush ();
    
    board->AO_BC_Load_A.writeRegister (numberOfBuffers - 1);
    board->AO_UC_Load_A.writeRegister (numberOfSamples - 1);     
    
    board->AO_Command_1.setAO_BC_Load (1);
    board->AO_Command_1.setAO_UC_Load (1);
    board->AO_Command_1.flush ();
    
    board->Joint_Reset.setAO_Configuration_End (1);
    board->Joint_Reset.flush ();   
    
    return;
}

void aoUpdate (tMSeries *board, 
                tMSeries::tAO_Mode_1::tAO_UPDATE_Source_Select source, 
                tMSeries::tAO_Mode_1::tAO_UPDATE_Source_Polarity polarity, 
                u32 periodDivisor)
{
    tBoolean internalUpdate;
    
    internalUpdate = (source == tMSeries::tAO_Mode_1::kAO_UPDATE_Source_SelectUI_TC);
    
    board->Joint_Reset.setAO_Configuration_Start (1);
    board->Joint_Reset.flush ();   
    
    board->AO_Command_2.setAO_BC_Gate_Enable (internalUpdate ? 0 : 1);
    board->AO_Command_2.flush ();
    
    board->AO_Mode_1.setAO_UPDATE_Source_Select (source);
    board->AO_Mode_1.setAO_UPDATE_Source_Polarity (polarity);
    board->AO_Mode_1.flush ();
    
    if (internalUpdate)
    {
        board->AO_UI_Load_A.writeRegister (1);
        board->AO_Command_1.writeAO_UI_Load (1);
        board->AO_UI_Load_A.writeRegister ( periodDivisor );
    }
    
    board->Joint_Reset.setAO_Configuration_End (1);
    board->Joint_Reset.flush ();
    
    return;        
}

void aoFifoMode (tMSeries *board, tBoolean fifoRetransmit)
{
    board->Joint_Reset.setAO_Configuration_Start (1);
    board->Joint_Reset.flush ();
    
    board->AO_Mode_2.setAO_FIFO_Retransmit_Enable (fifoRetransmit);
    board->AO_Mode_2.setAO_FIFO_Mode (tMSeries::tAO_Mode_2::kAO_FIFO_ModeLess_Than_Full);
    board->AO_Mode_2.flush ();
    
    board->AO_Personal.setAO_FIFO_Enable (1);
    board->AO_Personal.flush ();
    
    board->Joint_Reset.setAO_Configuration_End (1);
    board->Joint_Reset.flush ();
    
    return;        
}

void aoStop (tMSeries *board)
{
    board->Joint_Reset.setAO_Configuration_Start (1);
    board->Joint_Reset.flush ();
    
    board->AO_Mode_3.setAO_Stop_On_BC_TC_Error (0);
    board->AO_Mode_3.setAO_Stop_On_BC_TC_Trigger_Error (1);
    board->AO_Mode_3.setAO_Stop_On_Overrun_Error (1);
    board->AO_Mode_3.flush ();
    
    board->Joint_Reset.setAO_Configuration_End (1);
    board->Joint_Reset.flush ();
    
    return; 
}

void aoArm (tMSeries *board)
{
    board->AO_Mode_3.writeAO_Not_An_UPDATE (1);
    board->AO_Mode_3.writeAO_Not_An_UPDATE (0);
    
    while (board->Joint_Status_2.readAO_TMRDACWRs_In_Progress_St ())
    {
        // Wait
    }
        
    board->AO_Command_1.setAO_UI_Arm (1);
    board->AO_Command_1.setAO_UC_Arm (1);
    board->AO_Command_1.setAO_BC_Arm (1);
    board->AO_Command_1.flush ();
    
    return;
}

void aoStart (tMSeries *board)
{
    board->AO_Command_2.writeAO_START1_Pulse (1);
    
    return;
}

void aoDisarm (tMSeries *board)
{
    board->AO_Command_1.setAO_Disarm (1);
    board->AO_Command_1.flush ();
    
    return; 
}
